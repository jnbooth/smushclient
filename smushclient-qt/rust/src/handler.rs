use crate::convert::Convert;
use crate::ffi;
use cxx_qt_lib::QString;
use mud_transformer::{Output, OutputFragment, TextFragment};
use smushclient::SendRequest;
use std::pin::Pin;

pub struct ClientHandler<'a> {
    pub doc: Pin<&'a mut ffi::Document>,
}

impl<'a> ClientHandler<'a> {
    fn display_linebreak(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.doc.as_mut().appendLine() };
    }

    fn display_text(&mut self, fragment: TextFragment) {
        let text = QString::from(&*fragment.text);
        let style = fragment.flags.to_raw();
        let foreground = fragment.foreground.convert();
        let background = fragment.background.convert();
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.doc
                .as_mut()
                .appendText(&text, style, &foreground, &background);
        };
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

    fn send(&mut self, _request: SendRequest) {}
}
