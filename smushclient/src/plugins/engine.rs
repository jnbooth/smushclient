use core::str;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, BufReader};
use std::ops::{Deref, DerefMut};
use std::path::Path;

use super::effects::{AliasEffects, TriggerEffects};
use super::error::LoadError;
use super::error::LoadFailure;
use super::send::SendRequest;
use crate::handler::{Handler, SendHandler};
use crate::World;
use smushclient_plugins::{Alias, Matches, Pad, Plugin, PluginIndex, SendMatch, Sender, Trigger};

fn check_oneshot<T: AsRef<Sender>>(oneshots: &mut Vec<usize>, send: &SendMatch<T>) {
    if send.sender.as_ref().one_shot && oneshots.last() != Some(&send.index) {
        oneshots.push(send.index);
    }
}

fn delete_oneshots<T>(vec: &mut Vec<T>, oneshots: &[usize]) {
    for &pos in oneshots.iter().rev() {
        vec.remove(pos);
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
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
        }
    }

    pub fn load_plugins(&mut self, world: &World) -> Result<(), Vec<LoadFailure>> {
        self.plugins.clear();
        self.plugins.push(world.world_plugin());
        let errors: Vec<LoadFailure> = world
            .plugins
            .iter()
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

    pub fn set_world_plugin(&mut self, plugin: Plugin) {
        if let Some(world_plugin) = self
            .plugins
            .iter_mut()
            .find(|plugin| plugin.metadata.is_world_plugin)
        {
            *world_plugin = plugin;
        } else {
            self.plugins.push(plugin);
        }
        self.sort();
    }

    fn sort(&mut self) {
        self.plugins.sort_unstable();
    }

    pub fn alias<H: SendHandler>(
        &mut self,
        line: &str,
        world: &mut World,
        handler: &mut H,
    ) -> AliasEffects {
        let mut effects = AliasEffects::new();
        let mut oneshots = Vec::new();
        let mut text_buf = String::new();

        for (i, plugin) in self.plugins.iter_mut().enumerate() {
            if plugin.disabled {
                continue;
            }
            let aliases = if plugin.metadata.is_world_plugin {
                &mut world.aliases
            } else {
                &mut plugin.aliases
            };
            for send in Matches::find(aliases, line) {
                let alias: &Alias = send.sender;
                check_oneshot(&mut oneshots, &send);

                if send.has_send() {
                    handler.send(SendRequest {
                        pad: if alias.send_to.is_notepad() {
                            Some(Pad::new(alias, &plugin.metadata.name))
                        } else {
                            None
                        },
                        plugin: i,
                        line,
                        wildcards: send.wildcards(),
                        sender: alias,
                        text: send.text(&mut text_buf),
                    });
                }

                effects.add_effects(alias);

                if !alias.keep_evaluating {
                    delete_oneshots(aliases, &oneshots);
                    return effects;
                }
            }
            delete_oneshots(aliases, &oneshots);
        }

        effects
    }

    pub fn trigger<H: Handler>(
        &mut self,
        line: &str,
        world: &mut World,
        handler: &mut H,
    ) -> TriggerEffects {
        let mut effects = TriggerEffects::new();
        let mut oneshots = Vec::new();
        let mut text_buf = String::new();

        for (i, plugin) in self.plugins.iter_mut().enumerate() {
            if plugin.disabled {
                continue;
            }
            let triggers = if plugin.metadata.is_world_plugin {
                &mut world.triggers
            } else {
                &mut plugin.triggers
            };
            for send in Matches::find(triggers, line) {
                let trigger: &Trigger = send.sender;
                check_oneshot(&mut oneshots, &send);

                if send.has_send() {
                    handler.send(SendRequest {
                        pad: if trigger.send_to.is_notepad() {
                            Some(Pad::new(trigger, &plugin.metadata.name))
                        } else {
                            None
                        },
                        plugin: i,
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
                    delete_oneshots(triggers, &oneshots);
                    return effects;
                }
            }
            delete_oneshots(triggers, &oneshots);
        }

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

impl Deref for PluginEngine {
    type Target = [Plugin];

    fn deref(&self) -> &Self::Target {
        &self.plugins
    }
}

impl DerefMut for PluginEngine {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.plugins
    }
}
