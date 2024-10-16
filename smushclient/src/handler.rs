use crate::plugins::SendRequest;
use mud_transformer::Output;

pub trait SendHandler {
    fn display_error(&mut self, error: &str);
    fn send(&mut self, request: SendRequest);
}

pub trait Handler: SendHandler {
    fn display(&mut self, output: Output);
    fn permit_line(&mut self, line: &str) -> bool;
    fn play_sound(&mut self, path: &str);
}
