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
use super::iter::{ReactionIterable, Senders};
use crate::collections::LifetimeVec;
use crate::handler::{Handler, HandlerExt};
use crate::plugins::assert_unique_label;
use crate::world::World;
use crate::{SendIterable, SenderAccessError, SpanStyle};
use mud_transformer::Output;
use smushclient_plugins::{Alias, Pad, Plugin, PluginIndex, Trigger};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    aliases: LifetimeVec<(PluginIndex, usize, &'static Alias)>,
    triggers: LifetimeVec<(PluginIndex, usize, &'static Trigger)>,
    alias_buf: Vec<u8>,
    alias_matches: Vec<(PluginIndex, usize)>,
    trigger_buf: Vec<u8>,
    trigger_matches: Vec<(PluginIndex, usize)>,
    guards: Senders<SenderGuard>,
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
            aliases: LifetimeVec::new(),
            triggers: LifetimeVec::new(),
            alias_buf: Vec::new(),
            alias_matches: Vec::new(),
            trigger_buf: Vec::new(),
            trigger_matches: Vec::new(),
            guards: Senders::new(SenderGuard::new(), SenderGuard::new(), SenderGuard::new()),
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

    pub fn stop_evaluating<T: SendIterable>(&mut self) {
        self.guards.get_mut::<T>().stop();
    }

    pub fn alias<H: Handler>(
        &mut self,
        line: &str,
        source: CommandSource,
        world: &mut World,
        handler: &mut H,
    ) -> AliasEffects {
        let postpone = self.guards.get_mut::<Alias>();
        postpone.set_plugin_count(self.plugins.len());
        postpone.defer();
        let mut matched = self.aliases.acquire();
        let mut effects = AliasEffects::default();
        Alias::find_matches(
            postpone,
            &self.plugins,
            world,
            line,
            &mut matched,
            &mut effects,
        );

        if !effects.omit_from_output {
            handler.echo(line);
        }

        handler.send_all(&matched, line, &mut self.alias_buf, &self.plugins, postpone);
        let mut final_effects = AliasEffects::new(world, source);
        handler.send_all_scripts(&matched, line, &[], &mut final_effects, postpone);

        drop(matched);
        postpone.finalize::<Alias>(&mut self.plugins, world);

        final_effects
    }

    pub fn trigger<H: Handler>(
        &mut self,
        line: &str,
        output: &[Output],
        world: &mut World,
        handler: &mut H,
    ) -> TriggerEffects {
        let postpone = self.guards.get_mut::<Trigger>();
        postpone.set_plugin_count(self.plugins.len());
        postpone.defer();
        self.trigger_matches.clear();
        let mut effects = TriggerEffects::new();
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled {
                continue;
            }
            for (i, trigger) in Trigger::from_either(plugin, world).iter().enumerate() {
                if !trigger.enabled {
                    continue;
                }
                if postpone.stopped() {
                    postpone.set_stopped(false);
                    break;
                }
                let pad = if trigger.send_to.is_notepad() {
                    Some(Pad::new(trigger, &plugin.metadata.name))
                } else {
                    None
                };
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
                    handler.send(super::SendRequest {
                        plugin: plugin_index,
                        send_to: trigger.send_to,
                        text: trigger.expand_text(&mut self.trigger_buf, &captures),
                        pad,
                    });
                    if !trigger.repeats {
                        break;
                    }
                }
                if !matched {
                    continue;
                }
                if !trigger.script.is_empty() {
                    self.trigger_matches.push((plugin_index, i));
                }
                if !trigger.sound.is_empty() {
                    handler.play_sound(&trigger.sound);
                }
                effects.add_effects(trigger);
                trigger.unlock();
                if !trigger.keep_evaluating {
                    break;
                }
            }
        }

        for &(plugin_index, i) in &self.trigger_matches {
            let plugin = &self.plugins[plugin_index];
            let trigger = &Trigger::from_either(plugin, world)[i];
            trigger.lock();
            for captures in trigger.regex.captures_iter(line) {
                let Ok(captures) = captures else {
                    continue;
                };
                handler.send_script(super::SendScriptRequest {
                    plugin: plugin_index,
                    script: &trigger.script,
                    label: &trigger.label,
                    line,
                    regex: &trigger.regex,
                    wildcards: Some(captures),
                    output,
                });
            }
            trigger.unlock();
        }

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
