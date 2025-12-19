use core::str;
use std::collections::HashSet;
use std::io::{self};
use std::ops::{Deref, DerefMut};
use std::path::Path;
use std::{slice, vec};

use mud_transformer::Output;
use smushclient_plugins::{Alias, LoadError, Plugin, PluginIndex, SendTarget, Trigger};

use super::effects::CommandSource;
use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use super::error::LoadFailure;
use crate::client::PluginVariables;
use crate::handler::{Handler, HandlerExt};
use crate::world::World;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    alias_buf: String,
    trigger_buf: String,
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
            alias_buf: String::new(),
            trigger_buf: String::new(),
        }
    }

    pub fn load_plugins(&mut self, world: &World) -> Result<(), Vec<LoadFailure>> {
        self.plugins.clear();
        self.plugins.push(world.world_plugin());
        let errors: Vec<LoadFailure> = world
            .plugins
            .iter()
            .filter_map(|path| {
                let error = self.load_plugin(path).err()?;
                Some(LoadFailure {
                    error,
                    path: path.clone(),
                })
            })
            .collect();
        self.plugins.sort_unstable();
        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        let plugin = self.plugins.get_mut(index).ok_or(io::ErrorKind::NotFound)?;
        let path = plugin.metadata.path.clone();
        plugin.disabled = true;
        *plugin = Plugin::load(&path)?;
        let id = plugin.metadata.id.clone();
        self.plugins.sort_unstable();
        if self.plugins[index].metadata.id == id {
            return Ok(index);
        }
        #[allow(clippy::unwrap_used)]
        Ok(self
            .plugins
            .iter()
            .position(|plugin| plugin.metadata.id == id)
            .unwrap())
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
        self.plugins.sort_unstable();
        #[allow(clippy::unwrap_used)]
        Ok(self
            .plugins
            .iter()
            .enumerate()
            .find(|(_, plugin)| plugin.metadata.path == path)
            .unwrap())
    }

    fn load_plugin(&mut self, path: &Path) -> Result<&Plugin, LoadError> {
        let plugin = Plugin::load(path)?;
        self.plugins.push(plugin);
        #[allow(clippy::unwrap_used)]
        Ok(self.plugins.last().unwrap())
    }

    pub fn remove_plugin(&mut self, index: PluginIndex) -> Option<Plugin> {
        if index > self.plugins.len() {
            None
        } else {
            Some(self.plugins.remove(index))
        }
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
        self.plugins.sort_unstable();
    }

    pub fn alias<H: Handler>(
        &mut self,
        line: &str,
        source: CommandSource,
        world: &World,
        variables: &mut PluginVariables,
        handler: &mut H,
    ) -> AliasEffects {
        let mut effects = AliasEffects::new(world, source);

        if !world.enable_aliases {
            return effects;
        }

        let mut text_buf = String::new();
        let mut alias_buf = Alias::default();

        for (plugin_index, plugin) in self.plugins.iter_mut().enumerate() {
            if plugin.disabled {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            let aliases = if plugin.metadata.is_world_plugin {
                &world.aliases
            } else {
                &plugin.aliases
            };
            aliases.begin();
            while let Some(alias) = aliases.next() {
                let mut matched = false;
                let regex = {
                    let alias = alias.borrow();
                    if !alias.enabled {
                        continue;
                    }
                    alias.regex.clone()
                };
                for captures in regex.captures_iter(line) {
                    let Ok(captures) = captures else {
                        continue;
                    };
                    if !matched {
                        matched = true;
                    }
                    text_buf.clear();
                    let send_request = {
                        let alias = alias.borrow();
                        if alias.send_to == SendTarget::Variable {
                            variables.set_variable(
                                &plugin.metadata.id,
                                alias.variable.as_bytes().to_vec(),
                                alias
                                    .expand_text(&mut text_buf, &captures)
                                    .as_bytes()
                                    .to_vec(),
                            );
                            false
                        } else if enable_scripts || !alias.send_to.is_script() {
                            alias_buf.clone_from(&alias);
                            true
                        } else {
                            false
                        }
                    };
                    if send_request {
                        handler.send(super::SendRequest {
                            plugin: plugin_index,
                            send_to: alias_buf.send_to,
                            text: alias_buf.expand_text(&mut text_buf, &captures),
                            destination: alias_buf.destination(),
                        });
                    }
                    if !alias.borrow().repeats {
                        break;
                    }
                }
                if !matched {
                    continue;
                }
                let send_script = enable_scripts && {
                    let alias = alias.borrow();
                    if alias.script.is_empty() {
                        false
                    } else {
                        alias_buf.clone_from(&alias);
                        true
                    }
                };
                if send_script {
                    handler.send_scripts(plugin_index, &alias_buf, line, &[]);
                }
                let (one_shot, keep_evaluating) = {
                    let alias = alias.borrow();
                    effects.add_effects(&alias);
                    (alias.one_shot, alias.keep_evaluating)
                };
                if one_shot {
                    alias.remove();
                }
                if !keep_evaluating {
                    break;
                }
            }
            aliases.end();
        }

        effects
    }

    pub fn trigger<H: Handler>(
        &mut self,
        line: &str,
        output: &[Output],
        world: &World,
        variables: &mut PluginVariables,
        handler: &mut H,
    ) -> TriggerEffects {
        if !world.enable_triggers {
            return TriggerEffects::default();
        }

        let mut text_buf = String::new();
        let mut trigger_buf = Trigger::default();
        let mut effects = TriggerEffects::new();
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter_mut().enumerate() {
            if plugin.disabled {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            let triggers = if plugin.metadata.is_world_plugin {
                &world.triggers
            } else {
                &plugin.triggers
            };
            triggers.begin();
            while let Some(trigger) = triggers.next() {
                let mut matched = false;
                let regex = {
                    let trigger = trigger.borrow();
                    if !trigger.enabled {
                        continue;
                    }
                    trigger.regex.clone()
                };
                for captures in regex.captures_iter(line) {
                    let Ok(captures) = captures else {
                        continue;
                    };
                    if !matched {
                        matched = true;
                        style = SpanStyle::from(&*trigger.borrow());
                        has_style = !style.is_null();
                    }
                    if has_style && let Some(capture) = captures.get(0) {
                        handler.apply_styles(capture.start()..capture.end(), style);
                    }
                    self.trigger_buf.clear();
                    let send_request = {
                        let trigger = trigger.borrow();
                        if trigger.send_to == SendTarget::Variable {
                            variables.set_variable(
                                &plugin.metadata.id,
                                trigger.variable.as_bytes().to_vec(),
                                trigger
                                    .expand_text(&mut text_buf, &captures)
                                    .as_bytes()
                                    .to_vec(),
                            );
                            false
                        } else if enable_scripts || !trigger.send_to.is_script() {
                            trigger_buf.clone_from(&trigger);
                            true
                        } else {
                            false
                        }
                    };
                    if send_request {
                        handler.send(super::SendRequest {
                            plugin: plugin_index,
                            send_to: trigger_buf.send_to,
                            text: trigger_buf.expand_text(&mut text_buf, &captures),
                            destination: trigger_buf.destination(),
                        });
                    }
                    if !trigger.borrow().repeats {
                        break;
                    }
                }
                let send_script = enable_scripts && {
                    let trigger = trigger.borrow();
                    if trigger.script.is_empty() {
                        false
                    } else {
                        trigger_buf.clone_from(&trigger);
                        true
                    }
                };
                if send_script {
                    handler.send_scripts(plugin_index, &trigger_buf, line, output);
                }
                let (one_shot, keep_evaluating) = {
                    let trigger = trigger.borrow();
                    if !trigger.sound.is_empty() {
                        handler.play_sound(&trigger.sound);
                    }
                    effects.add_effects(&trigger);
                    (trigger.one_shot, trigger.keep_evaluating)
                };
                if one_shot {
                    trigger.remove();
                }
                if !keep_evaluating {
                    break;
                }
            }
            triggers.end();
        }

        if effects.omit_from_output {
            handler.erase_last_line();
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

impl<'a> IntoIterator for &'a mut PluginEngine {
    type Item = &'a mut Plugin;

    type IntoIter = slice::IterMut<'a, Plugin>;

    fn into_iter(self) -> Self::IntoIter {
        self.plugins.iter_mut()
    }
}
