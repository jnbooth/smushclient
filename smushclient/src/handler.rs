use std::ops::Range;

use crate::plugins::{SendRequest, SendScriptRequest, SpanStyle};
use crate::{SendIterable, World};
use mud_transformer::Output;
use smushclient_plugins::{Plugin, PluginIndex, Reaction};

pub trait Handler {
    fn apply_styles(&mut self, range: Range<usize>, style: SpanStyle);
    fn display(&mut self, output: &Output);
    fn display_error(&mut self, error: &str);
    fn erase_last_line(&mut self);
    fn send(&mut self, request: SendRequest);
    fn send_script(&mut self, request: SendScriptRequest);
    fn permit_line(&mut self, line: &str) -> bool;
    fn play_sound(&mut self, path: &str);
}

pub trait TimerHandler<T> {
    fn send_timer(&self, timer: &T);
    fn start_timer(&mut self, id: usize, milliseconds: u32);
}

pub trait HandlerExt {
    fn send_all_scripts<T>(
        &mut self,
        plugins: &[Plugin],
        world: &World,
        matches: &[(PluginIndex, usize)],
        line: &str,
        output: &[Output],
    ) where
        T: SendIterable + AsRef<Reaction>;
}

impl<H: Handler> HandlerExt for H {
    fn send_all_scripts<T>(
        &mut self,
        plugins: &[Plugin],
        world: &World,
        matches: &[(PluginIndex, usize)],
        line: &str,
        output: &[Output],
    ) where
        T: SendIterable + AsRef<Reaction>,
    {
        for &(plugin_index, i) in matches {
            let plugin = &plugins[plugin_index];
            let sender = &T::from_either(plugin, world)[i];
            let reaction: &Reaction = sender.as_ref();
            reaction.lock();
            for captures in reaction.regex.captures_iter(line) {
                let Ok(captures) = captures else {
                    continue;
                };
                self.send_script(super::SendScriptRequest {
                    plugin: plugin_index,
                    script: &reaction.script,
                    label: &reaction.label,
                    line,
                    regex: &reaction.regex,
                    wildcards: Some(captures),
                    output,
                });
            }
            reaction.unlock();
        }
    }
}
