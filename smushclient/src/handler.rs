use std::ops::Range;

use mud_transformer::Output;

use crate::plugins::{SendRequest, SendScriptRequest, SpanStyle};

pub trait Handler {
    fn apply_styles(&mut self, range: Range<usize>, style: SpanStyle);
    fn display(&mut self, output: &Output);
    fn display_error(&mut self, error: &str);
    fn echo(&mut self, input: &str);
    fn erase_last_line(&mut self);
    fn send(&mut self, request: SendRequest);
    fn send_script(&mut self, request: SendScriptRequest);
    fn permit_line(&mut self, line: &str) -> bool;
}
