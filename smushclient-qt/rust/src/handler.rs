use std::cell::RefCell;
use std::collections::HashSet;
use std::ops::Range;
use std::pin::Pin;

use cxx_qt_lib::QString;
use mud_transformer::Bytes;
use mud_transformer::opt::mssp;
use mud_transformer::opt::mxp;
use mud_transformer::output::{
    ControlFragment, EntityFragment, MxpFragment, Output, OutputFragment, TelnetFragment,
    TextFragment,
};
use mud_transformer::term::{self, CursorEffect};
use smushclient::{SendRequest, SendScriptRequest, SpanStyle, WorldConfig};

use crate::convert::Convert;
use crate::ffi::Document;
use crate::text_formatter::TextFormatter;

pub struct ClientHandler<'a> {
    doc: Pin<&'a mut Document>,
    formatter: &'a TextFormatter,
    carriage_return_clears_line: bool,
    no_echo_off: bool,
    stats: &'a RefCell<HashSet<String>>,
    had_output: bool,
}

impl Drop for ClientHandler<'_> {
    fn drop(&mut self) {
        self.doc.as_mut().end(self.had_output);
    }
}

impl<'a> ClientHandler<'a> {
    pub fn new(
        doc: Pin<&'a mut Document>,
        formatter: &'a TextFormatter,
        stats: &'a RefCell<HashSet<String>>,
        world: &WorldConfig,
    ) -> Self {
        doc.begin();
        Self {
            doc,
            formatter,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats,
            had_output: false,
        }
    }
}

impl ClientHandler<'_> {
    pub fn set_had_output(&mut self) {
        self.had_output = true;
    }

    fn display_text(&mut self, fragment: &TextFragment) {
        let text = QString::from(&*fragment.text);
        let format = self.formatter.text_format(fragment);
        match fragment.link.as_ref().and_then(|link| link.expire.as_ref()) {
            Some(expire) => self
                .doc
                .as_mut()
                .append_expiring_link(&text, &format, expire),
            None => self.doc.append_text(&text, &format),
        }
    }

    fn handle_control(&mut self, fragment: &ControlFragment) {
        match fragment {
            ControlFragment::Beep => self.doc.beep(),
            ControlFragment::Cursor(effect) => self.handle_cursor(*effect),
            ControlFragment::CarriageReturn if self.carriage_return_clears_line => {
                self.doc.erase_current_line();
            }
            ControlFragment::EraseCharacters(n) => self.handle_cursor(CursorEffect::Back(*n)),
            ControlFragment::Erase { target, range, .. } => self.handle_erase(*target, *range),
            ControlFragment::SetDynamicColor(dynamic, color) => self
                .doc
                .set_dynamic_color((*dynamic).into(), &color.convert()),
            ControlFragment::VerticalTab => self.handle_cursor(CursorEffect::Down(1)),
            _ => (),
        }
    }

    fn handle_cursor(&mut self, effect: CursorEffect) {
        use smushclient_qt_lib::QTextCursorMoveOperation as Move;

        match effect {
            CursorEffect::Back(n) => self.doc.move_cursor(Move::Left, n.into()),
            CursorEffect::Down(n) => self.doc.move_cursor(Move::Down, n.into()),
            CursorEffect::Forward(n) => self.doc.move_cursor(Move::Right, n.into()),
            CursorEffect::Index => self.doc.move_cursor(Move::Down, 1),
            CursorEffect::ReverseIndex => self.doc.move_cursor(Move::Up, 1),
            CursorEffect::Up(n) => self.doc.erase_characters(Move::Up, n.into()),
            _ => (),
        }
    }

    fn handle_erase(&mut self, target: term::EraseTarget, range: term::EraseRange) {
        if range == term::EraseRange::AfterCursor {
            return;
        }
        match target {
            term::EraseTarget::Display => self.doc.clear(),
            term::EraseTarget::Line => self.doc.erase_current_line(),
        }
    }

    fn handle_mxp(&mut self, fragment: &MxpFragment) {
        match fragment {
            MxpFragment::Entity(fragment) => self.handle_mxp_entity(fragment),
            MxpFragment::Expire(expires) => {
                let expires = expires.name.as_ref().map(AsRef::as_ref).unwrap_or_default();
                self.doc.as_mut().expire_links(expires);
            }
            MxpFragment::Stat(stat) => self.handle_mxp_stat(stat),
            MxpFragment::Variable(var) => self.doc.handle_mxp_variable(&var.name, &var.value),
            _ => (),
        }
    }

    fn handle_mxp_entity(&self, fragment: &EntityFragment) {
        let name = &*fragment.name;
        let value = fragment.value.as_deref().unwrap_or_default();
        self.doc.handle_mxp_entity(&format!("{name}={value}"));
        if self.stats.borrow().contains(name) {
            self.doc
                .update_mxp_stat(&QString::from(name), &QString::from(value));
        }
    }

    fn handle_mxp_stat(&self, stat: &mxp::Stat) {
        let entity = QString::from(&stat.entity);
        let max = match &stat.max {
            Some(max) => QString::from(max),
            None => QString::default(),
        };
        match &stat.caption {
            Some(caption) => self
                .doc
                .create_mxp_stat(&entity, &QString::from(caption), &max),
            None => self.doc.create_mxp_stat(&entity, &entity, &max),
        }
        let mut stats = self.stats.borrow_mut();
        if !stats.contains(&stat.entity) {
            stats.insert(stat.entity.clone());
        }
        if let Some(max) = &stat.max
            && !stats.contains(max)
        {
            stats.insert(max.clone());
        }
    }

    fn handle_subnegotiation(&mut self, code: u8, data: &Bytes) {
        self.doc.handle_telnet_subnegotiation(code, data);
        if code == mssp::OPT {
            for (variable, value) in mssp::decode(data) {
                self.doc.as_mut().handle_server_status(variable, value);
            }
        }
    }

    fn handle_telnet(&mut self, fragment: &TelnetFragment) {
        match fragment {
            TelnetFragment::GoAhead => self.doc.handle_telnet_go_ahead(),
            TelnetFragment::Mxp { enabled } => self.doc.handle_mxp_change(*enabled),
            TelnetFragment::Naws => self.doc.handle_telnet_naws(),
            TelnetFragment::Negotiation { source, verb, code } => {
                self.doc
                    .as_mut()
                    .handle_telnet_negotiation(*source, *verb, *code);
            }
            TelnetFragment::SetEcho { .. } if self.no_echo_off => (),
            TelnetFragment::SetEcho { should_echo } => self.doc.set_suppress_echo(!should_echo),
            TelnetFragment::Subnegotiation { code, data } => {
                self.handle_subnegotiation(*code, data);
            }
        }
    }
}

impl smushclient::Handler for ClientHandler<'_> {
    fn apply_styles(&mut self, range: Range<usize>, style: SpanStyle) {
        #[allow(clippy::cast_possible_truncation, clippy::cast_possible_wrap)]
        self.doc.apply_styles(
            range.start as i32,
            (range.end - range.start) as i32,
            &self.formatter.span_format(&style),
        );
    }

    fn display(&mut self, output: &Output) {
        match &output.fragment {
            OutputFragment::Control(effect) => self.handle_control(effect),
            OutputFragment::Hr => self.doc.append_html(&QString::from("<hr>")),
            OutputFragment::Image(_) => (),
            OutputFragment::LineBreak | OutputFragment::PageBreak => {
                self.doc.as_mut().append_line();
            }
            OutputFragment::Mxp(fragment) => self.handle_mxp(fragment),
            OutputFragment::Telnet(telnet) => self.handle_telnet(telnet),
            OutputFragment::Text(text) => self.display_text(text),
        }
    }

    fn display_error(&mut self, error: &str) {
        self.had_output = true;
        self.doc.as_mut().append_line();
        self.doc
            .append_text(&QString::from(error), self.formatter.error_format());
        self.doc.as_mut().append_line();
    }

    fn echo(&mut self, input: &str) {
        self.doc.echo(&QString::from(input));
    }

    fn erase_last_line(&mut self) {
        self.doc.erase_last_line();
    }

    fn permit_line(&mut self, line: &str) -> bool {
        self.doc.permit_line(line)
    }

    fn permit_sound(&mut self, file: &str) -> bool {
        self.doc.permit_sound(file)
    }

    fn send(&mut self, request: SendRequest) {
        self.doc.send(&request.into());
    }

    fn send_script(&mut self, request: SendScriptRequest) {
        self.doc.send_script(&request.into());
    }
}
