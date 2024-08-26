use crate::plugins::SendRequest;
use mud_transformer::Output;

pub trait SendHandler {
    fn send(&mut self, request: SendRequest);
}

pub trait Handler: SendHandler {
    fn display(&mut self, output: Output);
    fn play_sound(&mut self, path: &str);
}
