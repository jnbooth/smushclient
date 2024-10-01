use crate::adapter::{DocumentAdapter, QColorPair, SocketAdapter};
use crate::convert::Convert;
use cxx_qt_lib::{QByteArray, QString};
use mud_transformer::mxp::RgbColor;
use mud_transformer::{EntityFragment, Output, OutputFragment, TelnetFragment, TextFragment};
use smushclient::SendRequest;
use smushclient_plugins::SendTarget;
use std::collections::HashMap;

pub struct ClientHandler<'a> {
    pub doc: DocumentAdapter<'a>,
    pub socket: SocketAdapter<'a>,
    pub palette: &'a HashMap<RgbColor, i32>,
    pub send: &'a mut Vec<QString>,
}

const CUSTOM_FORMAT_INDEX: i32 = 0;
const ERROR_FORMAT_INDEX: i32 = 1;

impl<'a> ClientHandler<'a> {
    pub fn display_error(&mut self, error: &str) {
        self.doc
            .append_plaintext(&QString::from(error), ERROR_FORMAT_INDEX);
    }

    fn display_linebreak(&mut self) {
        self.doc.append_line();
    }

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
        let style = fragment.flags.to_raw();
        let colors = QColorPair {
            foreground: fragment.foreground.convert(),
            background: fragment.background.convert(),
        };
        match &fragment.action {
            Some(link) => {
                self.doc.append_link(&text, style, &colors, &link.into());
            }
            None => {
                self.doc.append_text(&text, style, &colors);
            }
        };
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
            self.doc
                .handle_mxp_variable(name.as_bytes(), value.as_bytes());
        }
        let entity = format!("{name}={value}");
        self.doc.handle_mxp_entity(entity.as_bytes());
    }

    fn handle_telnet(&self, fragment: &TelnetFragment) {
        match fragment {
            TelnetFragment::Subnegotiation { code, data } => self
                .doc
                .handle_telnet_subnegotiation(*code, &QByteArray::from(&**data)),
            TelnetFragment::IacGa => self.doc.handle_telnet_iac_ga(),
            TelnetFragment::Mxp { enabled } => self.doc.handle_mxp_change(*enabled),
            TelnetFragment::Do { code } => self.doc.handle_telnet_request(*code, true),
            TelnetFragment::Will { code } => self.doc.handle_telnet_request(*code, false),
            _ => (),
        }
    }

    pub fn output_sends(&mut self) {
        for send in self.send.iter() {
            self.doc.append_plaintext(send, CUSTOM_FORMAT_INDEX);
            self.doc.append_line();
        }
    }
}

impl<'a> smushclient::SendHandler for ClientHandler<'a> {
    fn send(&mut self, request: SendRequest) {
        if request.sender.send_to == SendTarget::Output {
            self.send.push(QString::from(request.text));
            return;
        }
        self.doc.send(
            request.sender.send_to.into(),
            request.plugin,
            &QString::from(request.text),
        );
    }
}

impl<'a> smushclient::Handler for ClientHandler<'a> {
    fn display(&mut self, output: Output) {
        match output.fragment {
            OutputFragment::LineBreak | OutputFragment::PageBreak => self.display_linebreak(),
            OutputFragment::MxpEntity(entity) => self.handle_mxp_entity(&entity),
            OutputFragment::Text(text) => self.display_text(&text),
            OutputFragment::Telnet(telnet) => self.handle_telnet(&telnet),
            _ => (),
        }
    }

    fn permit_line(&mut self, line: &str) -> bool {
        self.doc.permit_line(line.as_bytes())
    }

    fn play_sound(&mut self, _path: &str) {}
}
