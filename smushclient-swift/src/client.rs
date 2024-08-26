use mud_transformer::Output;
use std::vec;

use crate::bindings::ffi;

pub struct ClientHandler {
    output: Vec<ffi::OutputFragment>,
}

impl Default for ClientHandler {
    fn default() -> Self {
        Self::new()
    }
}

impl IntoIterator for ClientHandler {
    type Item = ffi::OutputFragment;

    type IntoIter = vec::IntoIter<Self::Item>;

    fn into_iter(self) -> Self::IntoIter {
        self.output.into_iter()
    }
}

impl ClientHandler {
    pub const fn new() -> Self {
        Self { output: Vec::new() }
    }
}

impl smushclient::SendHandler for ClientHandler {
    fn send(&mut self, request: smushclient::SendRequest) {
        self.output.push(ffi::OutputFragment::Send(request.into()));
    }
}

impl smushclient::Handler for ClientHandler {
    fn display(&mut self, output: Output) {
        if let Ok(fragment) = output.fragment.try_into() {
            self.output.push(fragment);
        }
    }

    fn play_sound(&mut self, path: &str) {
        self.output
            .push(ffi::OutputFragment::Sound(path.to_owned()));
    }
}
