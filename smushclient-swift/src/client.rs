use mud_transformer::OutputFragment;
use std::vec;

use crate::FfiOutputFragment;
use smushclient::SendRequest;

pub struct ClientHandler {
    output: Vec<FfiOutputFragment>,
}

impl Default for ClientHandler {
    fn default() -> Self {
        Self::new()
    }
}

impl IntoIterator for ClientHandler {
    type Item = FfiOutputFragment;

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

impl smushclient::Handler for ClientHandler {
    fn display(&mut self, fragment: OutputFragment) {
        self.output.push(fragment.into());
    }

    fn play_sound(&mut self, path: &str) {
        self.output.push(FfiOutputFragment::Sound(path.to_owned()));
    }

    fn send(&mut self, request: SendRequest) {
        self.output.push(FfiOutputFragment::Send(request.into()));
    }
}
