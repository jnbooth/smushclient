use crate::adapters::{DocumentAdapter, SocketAdapter};
use crate::convert::Convert;
use cxx_qt_lib::QString;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{Output, OutputFragment, TextFragment};
use smushclient::SendRequest;
use smushclient_plugins::SendTarget;
use std::collections::HashMap;
use std::io::Write;

pub struct ClientHandler<'a> {
    pub doc: DocumentAdapter<'a>,
    pub socket: SocketAdapter<'a>,
    pub palette: &'a HashMap<RgbColor, i32>,
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
            if let Some(index) = self.palette.get(&fragment.foreground) {
                self.doc.append_plaintext(&text, *index);
                return;
            }
        }
        let style = fragment.flags.to_raw();
        let foreground = fragment.foreground.convert();
        let background = fragment.background.convert();
        match fragment.action {
            Some(link) => {
                self.doc
                    .append_link(&text, style, &foreground, &background, &(&link).into());
            }
            None => {
                self.doc.append_text(&text, style, &foreground, &background);
            }
        };
    }
}

impl<'a> smushclient::SendHandler for ClientHandler<'a> {
    fn send(&mut self, request: SendRequest) {
        let text = request.text;
        match request.sender.send_to {
            SendTarget::World | SendTarget::WorldDelay | SendTarget::WorldImmediate => {
                if let Err(e) = self.socket.write_all(format!("{text}\r\n").as_bytes()) {
                    self.display_error(&e.to_string());
                }
            }
            SendTarget::Command => {
                self.doc.set_input(&QString::from(text));
            }
            SendTarget::Output => {
                self.doc
                    .append_plaintext(&QString::from(text), CUSTOM_FORMAT_INDEX);
            }
            SendTarget::Status => {
                self.doc.display_status_message(&QString::from(text));
            }
            _ => (),
        }
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
