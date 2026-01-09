use std::ops::Range;

use mud_transformer::Output;
use smushclient_plugins::{PluginIndex, Reaction};

use crate::plugins::{SendRequest, SendScriptRequest, SpanStyle};

pub trait Handler {
    fn apply_styles(&mut self, range: Range<usize>, style: SpanStyle);
    fn display(&mut self, output: &Output);
    fn display_error(&mut self, error: &str);
    fn erase_last_line(&mut self);
    fn send(&mut self, request: SendRequest);
    fn send_script(&mut self, request: SendScriptRequest);
    fn permit_line(&mut self, line: &str) -> bool;
}

pub trait HandlerExt {
    fn send_scripts(
        &mut self,
        index: PluginIndex,
        reaction: &Reaction,
        line: &str,
        output: &[Output],
    );
}

impl<H: Handler> HandlerExt for H {
    fn send_scripts(
        &mut self,
        index: PluginIndex,
        reaction: &Reaction,
        line: &str,
        output: &[Output],
    ) {
        if reaction.script.is_empty() {
            return;
        }
        for captures in reaction.regex.captures_iter(line) {
            let Ok(captures) = captures else {
                continue;
            };
            self.send_script(SendScriptRequest {
                plugin: index,
                script: &reaction.script,
                label: &reaction.label,
                line,
                regex: &reaction.regex,
                wildcards: Some(captures),
                output,
            });
        }
    }
}
