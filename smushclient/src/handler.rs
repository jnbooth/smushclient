use crate::plugins::{ReactionIterable, SendRequest, SendScriptRequest};
use mud_transformer::Output;
use smushclient_plugins::{Pad, PadSource, Plugin, PluginIndex, Reaction, SendMatchIterable};

pub trait Handler {
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
    fn send_all<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        text_buf: &mut String,
        plugins: &[Plugin],
    ) where
        T: ReactionIterable + PadSource;

    fn send_all_scripts<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        output: &[Output],
        effects: &mut T::Effects,
    ) where
        T: ReactionIterable;
}

impl<H: Handler> HandlerExt for H {
    fn send_all<T>(
        &mut self,
        senders: &[(PluginIndex, usize, &T)],
        line: &str,
        text_buf: &mut String,
        plugins: &[Plugin],
    ) where
        T: ReactionIterable + PadSource,
    {
        text_buf.clear();
        for &(plugin, index, sender) in senders {
            let reaction: &Reaction = sender.as_ref();
            if !reaction.enabled {
                continue;
            }
            reaction.lock();
            for send in sender.matches(plugin, index, line) {
                self.send(SendRequest {
                    plugin: send.plugin,
                    send_to: reaction.send_to,
                    text: send.text(text_buf),
                    pad: if reaction.send_to.is_notepad() {
                        Some(Pad::new(send.sender, &plugins[send.plugin].metadata.name))
                    } else {
                        None
                    },
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
    ) where
        T: ReactionIterable,
    {
        for &(plugin, index, sender) in senders {
            let reaction: &Reaction = sender.as_ref();
            if !reaction.enabled {
                continue;
            }
            sender.add_effects(effects);
            reaction.lock();
            for send in sender.matches(plugin, index, line) {
                self.send_script(SendScriptRequest {
                    plugin,
                    script: &reaction.script,
                    label: &reaction.label,
                    line,
                    output,
                    wildcards: send.captures,
                });
            }
            reaction.unlock();
        }
    }
}
