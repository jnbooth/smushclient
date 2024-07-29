use crate::plugins::SendRequest;
use mud_transformer::OutputFragment;
use smushclient_plugins::Occurrence;

pub trait Handler {
    fn display(&mut self, fragment: OutputFragment);
    fn play_sound(&mut self, path: &str);
    fn send(&mut self, request: SendRequest);
    fn set_timer(&mut self, occurrence: Occurrence, request: SendRequest);
}
