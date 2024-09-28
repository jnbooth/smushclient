use crate::plugins::SendRequest;
use mud_transformer::Output;

pub trait SendHandler {
    fn send(&mut self, request: SendRequest);
}

#[allow(unused_variables)]
pub trait Handler: SendHandler {
    fn display(&mut self, output: Output);
    fn permit_line(&mut self, line: &str) -> bool {
        true
    }
    fn play_sound(&mut self, path: &str);
}
