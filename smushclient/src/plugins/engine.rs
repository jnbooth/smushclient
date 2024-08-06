use core::str;
use std::collections::HashSet;

use super::effects::TriggerEffects;
use super::send::SendRequest;
use crate::handler::Handler;
use crate::plugins::effects::AliasEffects;
use smushclient_plugins::{Alias, Plugin, SendMatch, Sender, Senders, Trigger};

fn check_oneshot<T: AsRef<Sender>>(oneshots: &mut Vec<usize>, send: &SendMatch<T>) {
    if send.sender.as_ref().one_shot && oneshots.last() != Some(&send.pos) {
        oneshots.push(send.pos);
    }
}

fn is_world(plugin: &Plugin) -> bool {
    plugin.metadata.is_world_plugin
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    senders: Senders,
}

impl Default for PluginEngine {
    fn default() -> Self {
        Self::new()
    }
}

impl PluginEngine {
    pub const fn new() -> Self {
        Self {
            plugins: Vec::new(),
            senders: Senders::new(),
        }
    }

    pub fn load_plugins<I: IntoIterator<Item = Plugin>>(&mut self, iter: I) {
        self.plugins.extend(iter);
        self.sort();
    }

    pub fn set_world_plugin(&mut self, plugin: Option<Plugin>) {
        let plugin = match plugin {
            Some(plugin) => plugin,
            None => {
                if let Some(old_index) = self.plugins.iter().position(is_world) {
                    self.plugins.remove(old_index);
                }
                return;
            }
        };
        match self.plugins.iter_mut().find(|plugin| is_world(plugin)) {
            Some(old_plugin) => {
                let has_same_sequence = old_plugin.metadata.sequence == plugin.metadata.sequence;
                *old_plugin = plugin;
                if has_same_sequence {
                    return;
                }
            }
            None => self.plugins.push(plugin),
        }
        self.sort();
    }

    fn sort(&mut self) {
        self.senders.clear();
        self.plugins.sort_unstable();

        for (i, plugin) in self.plugins.iter_mut().enumerate() {
            self.senders.extend(i, plugin);
        }
    }

    pub fn alias<H: Handler>(&mut self, line: &str, handler: &mut H) -> AliasEffects {
        let mut effects = AliasEffects::new();
        let mut delete_oneshots = Vec::new();
        let mut text_buf = String::new();

        for send in self.senders.matches::<Alias>(line) {
            let alias: &Alias = send.sender;
            check_oneshot(&mut delete_oneshots, &send);

            if send.has_send() {
                handler.send(SendRequest {
                    pad: send.pad(&self.plugins),
                    plugin: send.plugin,
                    line,
                    wildcards: send.wildcards(),
                    sender: alias,
                    text: send.text(&mut text_buf),
                });
            }

            effects.add_effects(alias);

            if !alias.keep_evaluating {
                break;
            }
        }
        self.senders.delete_all::<Alias>(&delete_oneshots);

        effects
    }

    pub fn trigger<H: Handler>(&mut self, line: &str, handler: &mut H) -> TriggerEffects {
        let mut effects = TriggerEffects::new();
        let mut delete_oneshots = Vec::new();
        let mut text_buf = String::new();

        for send in self.senders.matches::<Trigger>(line) {
            let trigger: &Trigger = send.sender;
            check_oneshot(&mut delete_oneshots, &send);

            if send.has_send() {
                handler.send(SendRequest {
                    pad: send.pad(&self.plugins),
                    plugin: send.plugin,
                    line,
                    wildcards: send.wildcards(),
                    sender: trigger,
                    text: send.text(&mut text_buf),
                });
            }

            if !trigger.sound.is_empty() {
                handler.play_sound(&trigger.sound);
            }

            effects.add_effects(trigger);

            if !trigger.keep_evaluating {
                break;
            }
        }
        self.senders.delete_all::<Trigger>(&delete_oneshots);

        effects
    }

    pub fn supported_protocols(&self) -> HashSet<u8> {
        self.plugins
            .iter()
            .flat_map(|plugin| plugin.metadata.protocols.iter())
            .copied()
            .collect()
    }
}
