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
use super::output::is_nonvisual_output;
use crate::collections::LifetimeVec;
use crate::handler::{Handler, HandlerExt};
use crate::plugins::assert_unique_label;
use crate::world::World;
use crate::{SendIterable, SenderAccessError};
use mud_transformer::{Output, OutputFragment};
use smushclient_plugins::{Alias, Plugin, PluginIndex, Trigger};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    aliases: LifetimeVec<(PluginIndex, usize, &'static Alias)>,
    triggers: LifetimeVec<(PluginIndex, usize, &'static Trigger)>,
    alias_buf: String,
    trigger_buf: String,
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
            alias_buf: String::new(),
            trigger_buf: String::new(),
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
        Alias::find_matches(
            postpone,
            &self.plugins,
            world,
            line,
            &mut matched,
            &mut AliasEffects::default(),
        );

        handler.send_all(&matched, line, &mut self.alias_buf, &self.plugins);
        let mut final_effects = AliasEffects::new(world, source);
        handler.send_all_scripts(&matched, line, &[], &mut final_effects);

        drop(matched);
        postpone.finalize::<Alias>(&mut self.plugins, world);

        final_effects
    }

    pub fn trigger<H: Handler>(
        &mut self,
        line: &str,
        output: &mut [Output],
        world: &mut World,
        handler: &mut H,
    ) -> TriggerEffects {
        let postpone = self.guards.get_mut::<Trigger>();
        postpone.set_plugin_count(self.plugins.len());
        let mut matched = self.triggers.acquire();
        let mut effects = TriggerEffects::new();
        Trigger::find_matches(
            postpone,
            &self.plugins,
            world,
            line,
            &mut matched,
            &mut effects,
        );

        if !handler.permit_line(line) || effects.omit_from_output {
            for fragment in output.iter() {
                if is_nonvisual_output(&fragment.fragment) {
                    handler.display(fragment);
                }
            }
        } else {
            for fragment in output.iter_mut() {
                if let OutputFragment::Text(text) = &mut fragment.fragment {
                    effects.apply(text, world);
                }
                handler.display(fragment);
            }
        }

        handler.send_all(&matched, line, &mut self.trigger_buf, &self.plugins);
        let mut final_effects = TriggerEffects::new();
        handler.send_all_scripts(&matched, line, output, &mut final_effects);

        for &(_, _, trigger) in &matched {
            if trigger.enabled && !trigger.sound.is_empty() {
                handler.play_sound(&trigger.sound);
            }
        }

        drop(matched);
        postpone.finalize::<Trigger>(&mut self.plugins, world);

        if !effects.omit_from_output && final_effects.omit_from_output {
            handler.erase_last_line();
        }

        final_effects
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
