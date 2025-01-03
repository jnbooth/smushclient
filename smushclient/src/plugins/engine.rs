use core::str;
use std::collections::HashSet;
use std::io::{self};
use std::ops::{Deref, DerefMut};
use std::path::Path;
use std::{slice, vec};

use super::effects::CommandSource;
use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use super::error::LoadFailure;
use crate::client::PluginVariables;
use crate::handler::{Handler, HandlerExt};
use crate::world::World;
use mud_transformer::Output;
use smushclient_plugins::{LoadError, Plugin, PluginIndex, SendTarget};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    alias_buf: Vec<u8>,
    trigger_buf: Vec<u8>,
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
            alias_buf: Vec::new(),
            trigger_buf: Vec::new(),
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
        world: &mut World,
        variables: &mut PluginVariables,
        handler: &mut H,
    ) -> AliasEffects {
        let mut effects = AliasEffects::new(world, source);

        if !world.enable_aliases {
            return effects;
        }

        for (plugin_index, plugin) in self.plugins.iter_mut().enumerate() {
            if plugin.disabled {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            let aliases = if plugin.metadata.is_world_plugin {
                &mut world.aliases
            } else {
                &mut plugin.aliases
            };
            aliases.begin();
            while let Some(alias) = aliases.next() {
                if !alias.enabled {
                    continue;
                }
                let mut matched = false;
                for captures in alias.regex.captures_iter(line.as_bytes()) {
                    let Ok(captures) = captures else {
                        continue;
                    };
                    if !matched {
                        matched = true;
                    }
                    self.alias_buf.clear();
                    let text = alias.expand_text(&mut self.alias_buf, &captures);
                    if alias.send_to == SendTarget::Variable {
                        variables.set_variable(
                            &plugin.metadata.id,
                            alias.variable.clone(),
                            text.to_owned(),
                        );
                    } else if enable_scripts || !alias.send_to.is_script() {
                        handler.send(super::SendRequest {
                            plugin: plugin_index,
                            send_to: alias.send_to,
                            text,
                            destination: alias.destination(),
                        });
                    }
                    if !alias.repeats {
                        break;
                    }
                }
                if !matched {
                    continue;
                }
                if enable_scripts && !alias.script.is_empty() {
                    handler.send_scripts(plugin_index, alias, line, &[]);
                }
                effects.add_effects(alias);
                let keep_evaluating = alias.keep_evaluating;
                if alias.one_shot {
                    aliases.remove_current();
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
        world: &mut World,
        variables: &mut PluginVariables,
        handler: &mut H,
    ) -> TriggerEffects {
        if !world.enable_triggers {
            return TriggerEffects::default();
        }
        let mut effects = TriggerEffects::new();
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter_mut().enumerate() {
            if plugin.disabled {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            let triggers = if plugin.metadata.is_world_plugin {
                &mut world.triggers
            } else {
                &mut plugin.triggers
            };
            triggers.begin();
            while let Some(trigger) = triggers.next() {
                if !trigger.enabled {
                    continue;
                }
                let mut matched = false;
                for captures in trigger.regex.captures_iter(line.as_bytes()) {
                    let Ok(captures) = captures else {
                        continue;
                    };
                    if !matched {
                        matched = true;
                        style = SpanStyle::from(&*trigger);
                        has_style = !style.is_null();
                    }
                    if has_style {
                        if let Some(capture) = captures.get(0) {
                            handler.apply_styles(capture.start()..capture.end(), style);
                        }
                    }
                    self.trigger_buf.clear();
                    let text = trigger.expand_text(&mut self.trigger_buf, &captures);
                    if trigger.send_to == SendTarget::Variable {
                        variables.set_variable(
                            &plugin.metadata.id,
                            trigger.variable.clone(),
                            text.to_owned(),
                        );
                    } else if enable_scripts || !trigger.send_to.is_script() {
                        handler.send(super::SendRequest {
                            plugin: plugin_index,
                            send_to: trigger.send_to,
                            text,
                            destination: trigger.destination(),
                        });
                    }
                    if !trigger.repeats {
                        break;
                    }
                }
                if !matched {
                    continue;
                }
                if enable_scripts && !trigger.script.is_empty() {
                    handler.send_scripts(plugin_index, trigger, line, output);
                }
                if !trigger.sound.is_empty() {
                    handler.play_sound(&trigger.sound);
                }
                effects.add_effects(trigger);
                let keep_evaluating = trigger.keep_evaluating;
                if trigger.one_shot {
                    triggers.remove_current();
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
