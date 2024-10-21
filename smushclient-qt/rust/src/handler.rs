use crate::adapter::{DocumentAdapter, QColorPair};
use crate::convert::Convert;
use crate::sender::OutputSpan;
use cxx_qt_lib::{QByteArray, QString};
use mud_transformer::mxp::RgbColor;
use mud_transformer::{EntityFragment, Output, OutputFragment, TelnetFragment, TextFragment};
use smushclient::SendRequest;
use smushclient_plugins::SendTarget;
use std::collections::HashMap;

pub struct ClientHandler<'a> {
    pub doc: DocumentAdapter<'a>,
    pub palette: &'a HashMap<RgbColor, i32>,
    pub send: &'a mut Vec<QString>,
}

const CUSTOM_FORMAT_INDEX: i32 = 0;
const ERROR_FORMAT_INDEX: i32 = 1;

impl<'a> ClientHandler<'a> {
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
            self.doc.handle_mxp_variable(name, value);
        }
        let entity = format!("{name}={value}");
        self.doc.handle_mxp_entity(&entity);
    }

    fn handle_telnet(&self, fragment: &TelnetFragment) {
        match fragment {
            TelnetFragment::Subnegotiation { code, data } => self
                .doc
                .handle_telnet_subnegotiation(*code, &QByteArray::from(&**data)),
            TelnetFragment::IacGa => self.doc.handle_telnet_iac_ga(),
            TelnetFragment::Mxp { enabled } => self.doc.handle_mxp_change(*enabled),
            TelnetFragment::Will { code, supported } => {
                self.doc.handle_telnet_request(*code, *supported);
            }
            _ => (),
        }
    }

    pub fn output_sends(&mut self) {
        for send in self.send.iter() {
            self.doc.append_plaintext(send, CUSTOM_FORMAT_INDEX);
            self.doc.append_line();
        }
        self.send.clear();
    }
}

impl<'a> smushclient::SendHandler for ClientHandler<'a> {
    fn display_error(&mut self, error: &str) {
        self.doc
            .append_plaintext(&QString::from(error), ERROR_FORMAT_INDEX);
    }

    fn send(&mut self, request: SendRequest) {
        if !request.sender.script.is_empty() {
            self.doc.send_script(
                request.plugin,
                &QString::from(&request.sender.script),
                &QString::from(&request.sender.label),
                &QString::from(request.line),
                &request.wildcards.convert(),
                OutputSpan::cast(request.output),
            );
            return;
        }
        if request.sender.send_to == SendTarget::Output {
            self.send.push(QString::from(request.text));
            return;
        }
        self.doc.send(
            request.plugin,
            request.sender.send_to,
            &QString::from(request.text),
        );
    }
}

impl<'a> smushclient::Handler for ClientHandler<'a> {
    fn display(&mut self, output: Output) {
        match output.fragment {
            OutputFragment::Text(text) => self.display_text(&text),
            OutputFragment::LineBreak | OutputFragment::PageBreak => self.display_linebreak(),
            OutputFragment::MxpEntity(entity) => self.handle_mxp_entity(&entity),
            OutputFragment::Telnet(telnet) => self.handle_telnet(&telnet),
            OutputFragment::Hr => self.doc.append_html(&QString::from("<hr>")),
            _ => (),
        }
    }

    fn permit_line(&mut self, line: &str) -> bool {
        self.doc.permit_line(line)
    }

    fn play_sound(&mut self, path: &str) {
        self.doc.play_sound(&QString::from(path));
    }
}
