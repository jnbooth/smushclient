use crate::ffi;
use cxx_qt_lib::QString;
use mud_transformer::{Output, OutputFragment};
use smushclient::SendRequest;
use std::pin::Pin;

pub struct ClientHandler<'a> {
    pub doc: Pin<&'a mut ffi::Document>,
}

impl<'a> smushclient::Handler for ClientHandler<'a> {
    fn display(&mut self, output: Output) {
        if let OutputFragment::Text(fragment) = &output.fragment {
            unsafe {
                self.doc
                    .as_mut()
                    .appendText(&QString::from(&*fragment.text));
            };
        }
    }

    fn play_sound(&mut self, _path: &str) {}

    fn send(&mut self, _request: SendRequest) {}
}
