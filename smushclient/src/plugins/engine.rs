use core::str;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, BufReader};
use std::ops::{Deref, DerefMut};
use std::path::Path;

use super::effects::CommandSource;
use super::effects::{AliasEffects, TriggerEffects};
use super::error::LoadError;
use super::error::LoadFailure;
use super::guard::SenderGuard;
use super::iter::Senders;
use crate::client::PluginVariables;
use crate::handler::{Handler, HandlerExt};
use crate::plugins::assert_unique_label;
use crate::world::World;
use crate::{SendIterable, SenderAccessError, SpanStyle};
use mud_transformer::Output;
use smushclient_plugins::{Alias, Plugin, PluginIndex, SendTarget, Trigger};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    alias_buf: Vec<u8>,
    alias_matches: Vec<(PluginIndex, usize)>,
    trigger_buf: Vec<u8>,
    trigger_matches: Vec<(PluginIndex, usize)>,
    guards: Senders<SenderGuard>,
    stop_triggers: bool,
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
            alias_matches: Vec::new(),
            trigger_buf: Vec::new(),
            trigger_matches: Vec::new(),
            guards: Senders::new(SenderGuard::new(), SenderGuard::new(), SenderGuard::new()),
            stop_triggers: false,
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

    pub fn add_sender<'a, T: SendIterable>(
        &'a mut self,
        index: PluginIndex,
        world: &'a mut World,
        sender: T,
    ) -> Result<(usize, &'a T), SenderAccessError> {
        let senders = T::from_either_mut(&mut self.plugins[index], world);
        assert_unique_label(&sender, senders, None)?;
        Ok(self.guards.get_mut::<T>().add(index, senders, sender))
    }

    pub fn remove_sender<T: SendIterable, P: FnMut(&T) -> bool>(
        &mut self,
        index: PluginIndex,
        world: &mut World,
        pred: P,
    ) -> Option<T> {
        let senders = T::from_either_mut(&mut self.plugins[index], world);
        let pos = senders.iter().position(pred)?;
        self.guards.get_mut::<T>().remove::<T>(index, senders, pos)
    }

    pub fn remove_senders<T: SendIterable, P: FnMut(&T) -> bool>(
        &mut self,
        index: PluginIndex,
        world: &mut World,
        pred: P,
    ) -> usize {
        let senders = T::from_either_mut(&mut self.plugins[index], world);
        self.guards
            .get_mut::<T>()
            .remove_all::<T, P>(index, senders, pred)
    }

    pub fn stop_evaluating_triggers(&mut self) {
        self.stop_triggers = true;
    }

    pub fn alias<H: Handler>(
        &mut self,
        line: &str,
        source: CommandSource,
        world: &mut World,
        variables: &mut PluginVariables,
        handler: &mut H,
    ) -> AliasEffects {
        self.alias_matches.clear();
        let postpone = self.guards.get_mut::<Alias>();
        postpone.set_plugin_count(self.plugins.len());
        postpone.defer();
        let mut effects = AliasEffects::new(world, source);

        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            for (i, alias) in Alias::from_either(plugin, world).iter().enumerate() {
                if !alias.enabled {
                    continue;
                }
                let mut matched = false;
                for captures in alias.regex.captures_iter(line) {
                    let Ok(captures) = captures else {
                        continue;
                    };
                    if !matched {
                        matched = true;
                        alias.lock();
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
                    self.alias_matches.push((plugin_index, i));
                }
                effects.add_effects(alias);
                if alias.one_shot {
                    postpone.defer_remove(plugin_index, i);
                }
                alias.unlock();
                if !alias.keep_evaluating {
                    break;
                }
            }
        }

        handler.send_all_scripts::<Alias>(&self.plugins, world, &self.alias_matches, line, &[]);

        postpone.finalize::<Alias>(&mut self.plugins, world);

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
        self.trigger_matches.clear();
        let postpone = self.guards.get_mut::<Trigger>();
        postpone.set_plugin_count(self.plugins.len());
        postpone.defer();
        let mut effects = TriggerEffects::new();
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            self.stop_triggers = false;
            for (i, trigger) in Trigger::from_either(plugin, world).iter().enumerate() {
                if !trigger.enabled {
                    continue;
                }
                if self.stop_triggers {
                    break;
                }
                let mut matched = false;
                for captures in trigger.regex.captures_iter(line) {
                    let Ok(captures) = captures else {
                        continue;
                    };
                    if !matched {
                        matched = true;
                        style = SpanStyle::from(trigger);
                        has_style = !style.is_null();
                        trigger.lock();
                    }
                    if has_style {
                        if let Some(Some(capture)) = captures.iter().next() {
                            handler.apply_styles(capture.range(), style);
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
                    self.trigger_matches.push((plugin_index, i));
                }
                if !trigger.sound.is_empty() {
                    handler.play_sound(&trigger.sound);
                }
                if trigger.one_shot {
                    postpone.defer_remove(plugin_index, i);
                }
                effects.add_effects(trigger);
                trigger.unlock();
                if !trigger.keep_evaluating {
                    break;
                }
            }
        }

        handler.send_all_scripts::<Trigger>(
            &self.plugins,
            world,
            &self.trigger_matches,
            line,
            output,
        );

        postpone.finalize::<Trigger>(&mut self.plugins, world);

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
