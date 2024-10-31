use std::ops::Range;

use crate::plugins::{ReactionIterable, SendRequest, SendScriptRequest, SenderGuard, SpanStyle};
use mud_transformer::Output;
use smushclient_plugins::{Pad, PadSource, Plugin, PluginIndex, Reaction};

pub trait Handler {
    fn apply_styles(&mut self, range: Range<usize>, style: SpanStyle);
    fn display(&mut self, output: &Output);
    fn display_error(&mut self, error: &str);
    fn echo(&mut self, command: &str);
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
    fn send_all<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        text_buf: &mut Vec<u8>,
        plugins: &[Plugin],
        guard: &mut SenderGuard,
    ) where
        T: ReactionIterable + PadSource;

    fn send_all_scripts<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        output: &[Output],
        effects: &mut T::Effects,
        guard: &SenderGuard,
    ) where
        T: ReactionIterable;
}

impl<H: Handler> HandlerExt for H {
    fn send_all<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        text_buf: &mut Vec<u8>,
        plugins: &[Plugin],
        guard: &mut SenderGuard,
    ) where
        T: ReactionIterable + PadSource,
    {
        let mut skip_plugin = usize::MAX;
        for &(plugin, i, sender) in senders {
            if plugin == skip_plugin {
                continue;
            }
            let reaction: &Reaction = sender.as_ref();
            if !reaction.enabled || !reaction.has_send() {
                continue;
            }
            if guard.should_stop(plugin, i) {
                skip_plugin = plugin;
                continue;
            }
            reaction.lock();
            let mut found_capture = false;
            let pad = if reaction.send_to.is_notepad() {
                Some(Pad::new(sender, &plugins[plugin].metadata.name))
            } else {
                None
            };
            for captures in reaction.regex.captures_iter(line) {
                let Ok(captures) = captures else {
                    continue;
                };
                found_capture = true;
                text_buf.clear();
                self.send(SendRequest {
                    plugin,
                    send_to: reaction.send_to,
                    text: reaction.expand_text(text_buf, &captures),
                    pad,
                });
                if !reaction.repeats {
                    break;
                }
            }
            if !found_capture {
                self.send(SendRequest {
                    plugin,
                    send_to: reaction.send_to,
                    text: &reaction.text,
                    pad,
                });
            }
            reaction.unlock();
        }
    }

    fn send_all_scripts<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        output: &[Output],
        effects: &mut T::Effects,
        guard: &SenderGuard,
    ) where
        T: ReactionIterable,
    {
        let stops = guard.stops();
        let mut skip_plugin = usize::MAX;
        for &(plugin, i, sender) in senders {
            if plugin == skip_plugin {
                continue;
            }
            if stops[plugin] == i {
                skip_plugin = plugin;
                continue;
            }
            let reaction: &Reaction = sender.as_ref();
            if !reaction.enabled {
                continue;
            }
            sender.add_effects(effects);
            if reaction.script.is_empty() {
                continue;
            }
            reaction.lock();
            let mut found_capture = false;
            for captures in reaction.regex.captures_iter(line) {
                let Ok(captures) = captures else {
                    continue;
                };
                found_capture = true;
                self.send_script(SendScriptRequest {
                    plugin,
                    script: &reaction.script,
                    label: &reaction.label,
                    line,
                    output,
                    regex: &reaction.regex,
                    wildcards: Some(captures),
                });
                if !reaction.repeats {
                    break;
                }
            }
            if !found_capture {
                self.send_script(SendScriptRequest {
                    plugin,
                    script: &reaction.script,
                    label: &reaction.label,
                    line,
                    output,
                    regex: &reaction.regex,
                    wildcards: None,
                });
            }
            reaction.unlock();
        }
    }
}
