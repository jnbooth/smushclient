use crate::plugins::SendRequest;
use mud_transformer::Output;

pub trait Handler {
    fn display(&mut self, output: Output);
    fn play_sound(&mut self, path: &str);
    fn send(&mut self, request: SendRequest);
}
