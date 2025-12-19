use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::ops::Range;
use std::pin::Pin;

use cxx_qt_lib::{QByteArray, QString};
use mud_transformer::mxp::{self, RgbColor};
use mud_transformer::{
    EffectFragment, EntityFragment, Output, OutputFragment, TelnetFragment, TextFragment,
};
use smushclient::{AudioSinks, PlayMode, SendRequest, SendScriptRequest, SpanStyle};

use crate::colors::QColorPair;
use crate::convert::Convert;
use crate::ffi::Document;

pub struct ClientHandler<'a> {
    pub audio: &'a AudioSinks,
    pub doc: Pin<&'a mut Document>,
    pub palette: &'a HashMap<RgbColor, i32>,
    pub carriage_return_clears_line: bool,
    pub no_echo_off: bool,
    pub stats: &'a RefCell<HashSet<String>>,
}

const ERROR_FORMAT_INDEX: i32 = 1;

impl ClientHandler<'_> {
    fn display_text(&mut self, fragment: &TextFragment) {
        let text = QString::from(&*fragment.text);
        if fragment.flags.is_empty()
            && fragment.background == RgbColor::BLACK
            && fragment.action.is_none()
            && let Some(&index) = self.palette.get(&fragment.foreground)
        {
            self.doc.append_plaintext(&text, index);
            return;
        }
        let colors = QColorPair {
            foreground: fragment.foreground.convert(),
            background: fragment.background.convert(),
        };
        match &fragment.action {
            Some(link) => {
                self.doc
                    .as_mut()
                    .append_link(&text, fragment.flags, &colors, &link.into());
            }
            None => {
                self.doc.append_text(&text, fragment.flags, &colors);
            }
        }
    }

    fn handle_effect(&mut self, fragment: &EffectFragment) {
        match fragment {
            EffectFragment::Backspace | EffectFragment::EraseCharacter => {
                self.doc.erase_last_character();
            }
            EffectFragment::Beep => self.doc.beep(),
            EffectFragment::CarriageReturn if !self.carriage_return_clears_line => (),
            EffectFragment::CarriageReturn | EffectFragment::EraseLine => {
                self.doc.erase_current_line();
            }
            EffectFragment::ExpireLinks(None) => self.doc.as_mut().expire_links(""),
            EffectFragment::ExpireLinks(Some(expires)) => {
                self.doc.as_mut().expire_links(expires.as_str());
            }
            EffectFragment::StatusBar(stat) => self.handle_mxp_stat(stat),
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
        if self.stats.borrow().contains(name) {
            self.doc
                .update_mxp_stat(&QString::from(name), &QString::from(value));
        }
    }

    fn handle_mxp_stat(&self, stat: &mxp::Stat) {
        let entity = QString::from(&stat.entity);
        let max = stat.max.convert();
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
            TelnetFragment::ServerStatus { variable, value } => self
                .doc
                .as_mut()
                .handle_server_status(&QByteArray::from(&**variable), &QByteArray::from(&**value)),
            TelnetFragment::SetEcho { .. } if self.no_echo_off => (),
            TelnetFragment::SetEcho { should_echo } => self.doc.set_suppress_echo(!should_echo),
            TelnetFragment::Subnegotiation { code, data } => self
                .doc
                .handle_telnet_subnegotiation(*code, &QByteArray::from(&**data)),
        }
    }
}

impl smushclient::Handler for ClientHandler<'_> {
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
            OutputFragment::LineBreak | OutputFragment::PageBreak => {
                self.doc.as_mut().append_line();
            }
            OutputFragment::MxpEntity(entity) => self.handle_mxp_entity(entity),
            OutputFragment::MxpError(error) => eprintln!("MXP error: {error}"),
            OutputFragment::Telnet(telnet) => self.handle_telnet(telnet),
            OutputFragment::Text(text) => self.display_text(text),
            OutputFragment::Frame(_) | OutputFragment::Image(_) => (),
        }
    }

    fn display_error(&mut self, error: &str) {
        self.doc.as_mut().append_line();
        self.doc
            .append_plaintext(&QString::from(error), ERROR_FORMAT_INDEX);
        self.doc.as_mut().append_line();
    }

    fn erase_last_line(&mut self) {
        self.doc.erase_last_line();
    }

    fn permit_line(&mut self, line: &str) -> bool {
        self.doc.permit_line(line)
    }

    fn play_sound(&mut self, path: &str) {
        if let Err(e) = self.audio.play_file(0, path, 1.0, PlayMode::Once) {
            self.display_error(&e.to_string());
        }
    }

    fn send(&mut self, request: SendRequest) {
        self.doc.send(&request.into());
    }

    fn send_script(&mut self, request: SendScriptRequest) {
        self.doc.send_script(&request.into());
    }
}
