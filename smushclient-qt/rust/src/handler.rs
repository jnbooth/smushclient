use crate::adapter::{DocumentAdapter, QColorPair, SocketAdapter};
use crate::convert::Convert;
use cxx_qt_lib::QString;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{Output, OutputFragment, TextFragment};
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

    fn display_text(&mut self, fragment: TextFragment) {
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
        match fragment.action {
            Some(link) => {
                self.doc.append_link(&text, style, &colors, &(&link).into());
            }
            None => {
                self.doc.append_text(&text, style, &colors);
            }
        };
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
            OutputFragment::Text(text) => self.display_text(text),
            _ => (),
        }
    }

    fn play_sound(&mut self, _path: &str) {}
}
