use crate::plugins::SendRequest;
use mud_transformer::OutputFragment;

pub trait Handler {
    fn display(&mut self, fragment: OutputFragment);
    fn play_sound(&mut self, path: &str);
    fn send(&mut self, request: SendRequest);
}
