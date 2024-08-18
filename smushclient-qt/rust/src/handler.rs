use mud_transformer::{Output, OutputFragment};
use smushclient::SendRequest;
use std::ops::{Deref, DerefMut};

#[derive(Debug, Default)]
pub struct ClientHandler {
    inner: Vec<OutputFragment>,
}

impl Deref for ClientHandler {
    type Target = Vec<OutputFragment>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl DerefMut for ClientHandler {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

impl smushclient::Handler for ClientHandler {
    fn display(&mut self, output: Output) {
        self.inner.push(output.fragment);
    }

    fn play_sound(&mut self, _path: &str) {}

    fn send(&mut self, _request: SendRequest) {}
}
