use core::str;
use std::collections::HashSet;
use std::{slice, vec};

use super::effects::TriggerEffects;
use super::send::SendRequest;
use crate::handler::{Handler, SendHandler};
use crate::plugins::effects::AliasEffects;
use smushclient_plugins::{Alias, Plugin, SendMatch, Sendable, Sender, Senders, Trigger};

fn check_oneshot<T: AsRef<Sender>>(oneshots: &mut Vec<usize>, send: &SendMatch<T>) {
    if send.sender.as_ref().one_shot && oneshots.last() != Some(&send.pos) {
        oneshots.push(send.pos);
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    senders: Senders,
    world_plugin_index: Option<usize>,
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
            world_plugin_index: None,
        }
    }

    pub fn load_plugins<I: IntoIterator<Item = Plugin>>(&mut self, iter: I) {
        self.plugins.extend(iter);
        self.sort();
    }

    pub fn set_world_plugin(&mut self, plugin: Option<Plugin>) {
        let Some(plugin) = plugin else {
            if let Some(index) = self.world_plugin_index {
                self.plugins.remove(index);
                self.world_plugin_index = None;
            }
            return;
        };
        if let Some(index) = self.world_plugin_index {
            self.plugins[index] = plugin;
            return;
        }
        self.plugins.push(plugin);
        self.sort();
    }

    fn sort(&mut self) {
        self.senders.clear();
        if self.plugins.is_empty() {
            return;
        }
        self.plugins.sort_unstable();
        for (i, plugin) in self.plugins.iter_mut().enumerate() {
            self.senders.extend(i, plugin);
            if plugin.metadata.is_world_plugin {
                self.world_plugin_index = Some(i);
            }
        }
        self.senders.sort();
    }

    pub fn alias<H: SendHandler>(&mut self, line: &str, handler: &mut H) -> AliasEffects {
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

    pub fn iter(&self) -> <&Self as IntoIterator>::IntoIter {
        self.into_iter()
    }

    pub fn find_by<'a, T, P>(&'a self, predicate: P) -> impl Iterator<Item = &'a T> + 'a
    where
        T: Sendable,
        P: FnMut(&T) -> bool + 'a,
    {
        T::indexer(&self.senders).find_by(predicate)
    }

    pub fn find_by_mut<'a, T, P>(&'a mut self, predicate: P) -> impl Iterator<Item = &'a mut T> + 'a
    where
        T: Sendable,
        P: FnMut(&T) -> bool + 'a,
    {
        T::indexer_mut(&mut self.senders).find_by_mut(predicate)
    }
}

impl IntoIterator for PluginEngine {
    type Item = Plugin;

    type IntoIter = vec::IntoIter<Plugin>;

    fn into_iter(self) -> Self::IntoIter {
        self.plugins.into_iter()
    }
}

impl<'a> IntoIterator for &'a PluginEngine {
    type Item = &'a Plugin;

    type IntoIter = slice::Iter<'a, Plugin>;

    fn into_iter(self) -> Self::IntoIter {
        self.plugins.iter()
    }
}
