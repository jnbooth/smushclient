use crate::adapter::{DocumentAdapter, QColorPair};
use crate::convert::Convert;
use cxx_qt_lib::{QByteArray, QString};
use mud_transformer::mxp::RgbColor;
use mud_transformer::{
    EffectFragment, EntityFragment, Output, OutputFragment, TelnetFragment, TextFragment,
};
use smushclient::{SendRequest, SendScriptRequest, SpanStyle};
use std::collections::HashMap;
use std::ops::Range;

pub struct ClientHandler<'a> {
    pub doc: DocumentAdapter<'a>,
    pub palette: &'a HashMap<RgbColor, i32>,
    pub carriage_return_clears_line: bool,
    pub no_echo_off: bool,
}

const ERROR_FORMAT_INDEX: i32 = 1;

impl<'a> ClientHandler<'a> {
    fn display_text(&mut self, fragment: &TextFragment) {
        let text = QString::from(&*fragment.text);
        if fragment.flags.is_empty()
            && fragment.background == RgbColor::BLACK
            && fragment.action.is_none()
        {
            if let Some(&index) = self.palette.get(&fragment.foreground) {
                self.doc.append_plaintext(&text, index);
                return;
            }
        }
        let colors = QColorPair {
            foreground: fragment.foreground.convert(),
            background: fragment.background.convert(),
        };
        match &fragment.action {
            Some(link) => {
                self.doc
                    .append_link(&text, fragment.flags, &colors, &link.into());
            }
            None => {
                self.doc.append_text(&text, fragment.flags, &colors);
            }
        };
    }

    fn handle_effect(&self, fragment: &EffectFragment) {
        match fragment {
            EffectFragment::Backspace | EffectFragment::EraseCharacter => {
                self.doc.erase_last_character();
            }
            EffectFragment::Beep => self.doc.beep(),
            EffectFragment::CarriageReturn if !self.carriage_return_clears_line => (),
            EffectFragment::CarriageReturn | EffectFragment::EraseLine => {
                self.doc.erase_current_line();
            }
            _ => (),
        }
    }

    fn handle_mxp_entity(&self, fragment: &EntityFragment) {
        let EntityFragment::Set {
            name,
            value,
            publish,
            is_variable,
        } = fragment
        else {
            return;
        };
        if !publish {
            return;
        }
        if *is_variable {
            self.doc.handle_mxp_variable(name, value);
        }
        let entity = format!("{name}={value}");
        self.doc.handle_mxp_entity(&entity);
    }

    fn handle_telnet(&self, fragment: &TelnetFragment) {
        match fragment {
            TelnetFragment::IacGa => self.doc.handle_telnet_iac_ga(),
            TelnetFragment::Mxp { enabled } => self.doc.handle_mxp_change(*enabled),
            TelnetFragment::Naws => self.doc.handle_telnet_naws(),
            TelnetFragment::Negotiation { source, verb, code } => {
                self.doc.handle_telnet_negotiation(*source, *verb, *code);
            }
            TelnetFragment::SetEcho { .. } if self.no_echo_off => (),
            TelnetFragment::SetEcho { should_echo } => self.doc.set_suppress_echo(!should_echo),
            TelnetFragment::Subnegotiation { code, data } => self
                .doc
                .handle_telnet_subnegotiation(*code, &QByteArray::from(&**data)),
        }
    }
}

impl<'a> smushclient::Handler for ClientHandler<'a> {
    fn apply_styles(&mut self, range: Range<usize>, style: SpanStyle) {
        self.doc.apply_styles(
            range,
            style.flags,
            &QColorPair {
                foreground: style.foreground.convert(),
                background: style.background.convert(),
            },
        );
    }

    fn display(&mut self, output: &Output) {
        match &output.fragment {
            OutputFragment::Effect(effect) => self.handle_effect(effect),
            OutputFragment::Hr => self.doc.append_html(&QString::from("<hr>")),
            OutputFragment::LineBreak | OutputFragment::PageBreak => self.doc.append_line(),
            OutputFragment::MxpEntity(entity) => self.handle_mxp_entity(entity),
            OutputFragment::MxpError(error) => eprintln!("MXP error: {error}"),
            OutputFragment::Telnet(telnet) => self.handle_telnet(telnet),
            OutputFragment::Text(text) => self.display_text(text),
            OutputFragment::Frame(_) | OutputFragment::Image(_) => (),
        }
    }

    fn display_error(&mut self, error: &str) {
        self.doc.append_line();
        self.doc
            .append_plaintext(&QString::from(error), ERROR_FORMAT_INDEX);
        self.doc.append_line();
    }

    fn erase_last_line(&mut self) {
        self.doc.erase_last_line();
    }

    fn permit_line(&mut self, line: &str) -> bool {
        self.doc.permit_line(line)
    }

    fn play_sound(&mut self, path: &str) {
        self.doc.play_sound(&QString::from(path));
    }

    fn send(&mut self, request: SendRequest) {
        self.doc.send(&request.into());
    }

    fn send_script(&mut self, request: SendScriptRequest) {
        self.doc.send_script(&request.into());
    }
}
