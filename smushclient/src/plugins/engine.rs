use core::str;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, BufReader};
use std::ops::{Deref, DerefMut};
use std::path::Path;

use super::effects::{AliasEffects, TriggerEffects};
use super::error::LoadError;
use super::error::LoadFailure;
use crate::collections::LifetimeVec;
use crate::handler::{Handler, HandlerExt};
use crate::plugins::iter::ReactionIterable;
use crate::plugins::output::is_nonvisual_output;
use crate::{SendIterable, World};
use mud_transformer::{Output, OutputFragment};
use smushclient_plugins::{Alias, Plugin, PluginIndex, Trigger};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct PluginEngine {
    plugins: Vec<Plugin>,
    aliases: LifetimeVec<(PluginIndex, usize, &'static Alias)>,
    triggers: LifetimeVec<(PluginIndex, usize, &'static Trigger)>,
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
            aliases: LifetimeVec::new(),
            triggers: LifetimeVec::new(),
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

    fn remove_oneshots<T: SendIterable>(
        &mut self,
        oneshots: &[(PluginIndex, usize)],
        world: &mut World,
    ) {
        let mut current_plugin = usize::MAX;
        let mut senders: &mut Vec<T> = &mut Vec::new();
        for &(next_plugin, i) in oneshots.iter().rev() {
            if next_plugin != current_plugin {
                let plugin = &mut self.plugins[next_plugin];
                current_plugin = next_plugin;
                senders = if plugin.metadata.is_world_plugin {
                    T::from_world_mut(world)
                } else {
                    T::from_plugin_mut(plugin)
                }
            }
            senders.remove(i);
        }
    }

    pub fn alias<H: Handler>(
        &mut self,
        line: &str,
        world: &mut World,
        handler: &mut H,
    ) -> AliasEffects {
        let mut oneshots = Vec::new();
        let mut matched = self.aliases.acquire();
        let effects = Alias::find_matches(&self.plugins, world, line, &mut matched, &mut oneshots);

        if !effects.omit_from_output {
            handler.echo(line);
        }

        handler.send_all(&matched, line, &mut self.alias_buf, &self.plugins);
        let final_effects = handler.send_all_scripts(&matched, line, &[]);

        drop(matched);
        self.remove_oneshots::<Alias>(&oneshots, world);

        final_effects
    }

    pub fn trigger<H: Handler>(
        &mut self,
        line: &str,
        output: &mut [Output],
        world: &mut World,
        handler: &mut H,
    ) -> TriggerEffects {
        let mut oneshots = Vec::new();
        let mut matched = self.triggers.acquire();
        let effects =
            Trigger::find_matches(&self.plugins, world, line, &mut matched, &mut oneshots);

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
        let final_effects = handler.send_all_scripts(&matched, line, output);

        for &(_, _, trigger) in &matched {
            if trigger.enabled && !trigger.sound.is_empty() {
                handler.play_sound(&trigger.sound);
            }
        }

        drop(matched);
        self.remove_oneshots::<Trigger>(&oneshots, world);

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
