use mud_transformer::Output;
use std::vec;

use crate::ffi;

pub struct AliasHandler {
    requests: Vec<ffi::SendRequest>,
}

impl Default for AliasHandler {
    fn default() -> Self {
        Self::new()
    }
}

impl From<AliasHandler> for Vec<ffi::SendRequest> {
    fn from(value: AliasHandler) -> Self {
        value.requests
    }
}

impl IntoIterator for AliasHandler {
    type Item = ffi::SendRequest;

    type IntoIter = vec::IntoIter<Self::Item>;

    fn into_iter(self) -> Self::IntoIter {
        self.requests.into_iter()
    }
}

impl AliasHandler {
    pub const fn new() -> Self {
        Self {
            requests: Vec::new(),
        }
    }
}

impl smushclient::Handler for AliasHandler {
    fn display(&mut self, _output: &Output) {}

    fn display_error(&mut self, error: &str) {
        eprintln!("Handler error: {error}");
    }

    fn erase_last_line(&mut self) {}

    fn send(&mut self, request: smushclient::SendRequest) {
        self.requests.push(request.into());
    }

    fn send_script(&mut self, _request: smushclient::SendScriptRequest) {}

    fn permit_line(&mut self, _line: &str) -> bool {
        true
    }

    fn play_sound(&mut self, _path: &str) {}
}

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

impl smushclient::Handler for ClientHandler {
    fn display(&mut self, output: &Output) {
        if let Ok(fragment) = output.fragment.clone().try_into() {
            self.output.push(fragment);
        }
    }

    fn display_error(&mut self, error: &str) {
        eprintln!("Handler error: {error}");
    }

    fn erase_last_line(&mut self) {}

    fn play_sound(&mut self, path: &str) {
        self.output
            .push(ffi::OutputFragment::Sound(path.to_owned()));
    }

    fn permit_line(&mut self, _line: &str) -> bool {
        true
    }

    fn send(&mut self, request: smushclient::SendRequest) {
        self.output.push(ffi::OutputFragment::Send(request.into()));
    }

    fn send_script(&mut self, _request: smushclient::SendScriptRequest) {}
}
