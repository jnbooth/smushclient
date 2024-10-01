use core::str;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, BufReader};
use std::path::Path;
use std::{slice, vec};

use super::effects::TriggerEffects;
use super::error::LoadError;
use super::send::SendRequest;
use crate::handler::{Handler, SendHandler};
use crate::plugins::effects::AliasEffects;
use crate::plugins::LoadFailure;
use smushclient_plugins::{
    Alias, Indexer, Plugin, PluginIndex, SendMatch, Sendable, Sender, Senders, Trigger,
};

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

    pub fn plugin_count(&self) -> usize {
        self.plugins.len()
    }

    pub fn load_plugins<I>(&mut self, iter: I) -> Result<(), Vec<LoadFailure>>
    where
        I: IntoIterator,
        I::Item: AsRef<Path>,
    {
        let errors: Vec<LoadFailure> = iter
            .into_iter()
            .filter_map(|path| {
                let path = path.as_ref();
                let error = self.load_plugin(path).err()?;
                Some(LoadFailure {
                    error,
                    path: path.to_path_buf(),
                })
            })
            .collect();
        self.sort();
        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }

    pub fn add_plugin<P: AsRef<Path>>(
        &mut self,
        path: P,
    ) -> Result<(PluginIndex, &Plugin), LoadError> {
        let path = path.as_ref();
        if self
            .plugins
            .iter()
            .any(|plugin| plugin.metadata.path == path)
        {
            return Err(io::Error::from(io::ErrorKind::AlreadyExists).into());
        }
        self.load_plugin(path)?;
        self.sort();
        Ok(self
            .plugins
            .iter()
            .enumerate()
            .find(|(_, plugin)| plugin.metadata.path == path)
            .unwrap())
    }

    fn load_plugin(&mut self, path: &Path) -> Result<&Plugin, LoadError> {
        let file = File::open(path)?;
        let reader = BufReader::new(file);
        let mut plugin = Plugin::from_xml(reader)?;

        plugin.metadata.path = path.to_path_buf();
        self.plugins.push(plugin);
        Ok(self.plugins.last().unwrap())
    }

    pub fn remove_plugin(&mut self, id: &str) -> Option<Plugin> {
        let index = self
            .plugins
            .iter()
            .enumerate()
            .find(|(_, plugin)| plugin.metadata.id == id)?
            .0;
        Some(self.plugins.remove(index))
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

    pub fn disable_plugin(&mut self, index: PluginIndex) -> bool {
        let Some(plugin) = self.plugins.get_mut(index) else {
            return false;
        };
        if plugin.disabled {
            return true;
        }
        self.senders.remove_by_plugin(index);
        true
    }

    pub fn enable_plugin(&mut self, index: PluginIndex) -> bool {
        let Some(plugin) = self.plugins.get_mut(index) else {
            return false;
        };
        if !plugin.disabled {
            return true;
        }
        plugin.disabled = false;
        self.senders.extend(index, plugin);
        self.senders.sort();
        true
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

    pub fn plugin(&self, index: PluginIndex) -> Option<&Plugin> {
        self.plugins.get(index)
    }

    pub fn indexer<T: Sendable>(&self) -> &Indexer<T> {
        T::indexer(&self.senders)
    }

    pub fn indexer_mut<T: Sendable>(&mut self) -> &mut Indexer<T> {
        T::indexer_mut(&mut self.senders)
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
