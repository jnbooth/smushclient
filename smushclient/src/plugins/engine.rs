use std::collections::HashSet;
use std::io::{self};
use std::ops::Deref;
use std::path::Path;
use std::{iter, slice, vec};

use smushclient_plugins::{CursorVec, LoadError, Plugin, PluginIndex};

use super::error::LoadFailure;
use super::iter::SendIterable;
use crate::world::WorldConfig;

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct PluginEngine {
    plugins: Vec<Plugin>,
    world_script_index: Option<usize>,
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
            world_script_index: None,
        }
    }

    fn find_world_plugin(&mut self) {
        self.world_script_index = self
            .plugins
            .iter()
            .position(|plugin| plugin.metadata.is_world_plugin);
    }

    #[allow(clippy::type_complexity)]
    pub fn all_senders<T: SendIterable>(
        &self,
    ) -> iter::Map<slice::Iter<'_, Plugin>, fn(&Plugin) -> &CursorVec<T>> {
        self.plugins.iter().map(Plugin::senders)
    }

    pub(crate) fn load_plugins(&mut self, world: &WorldConfig) -> Result<(), Vec<LoadFailure>> {
        self.plugins
            .retain(|plugin| plugin.metadata.is_world_plugin);
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
        self.find_world_plugin();
        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        let plugin = &mut self.plugins[index];
        let path = plugin.metadata.path.clone();
        plugin.disabled.set(true);
        *plugin = Plugin::load(&path)?;
        let id = plugin.metadata.id.clone();
        self.plugins.sort_unstable();
        self.find_world_plugin();
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
        self.find_world_plugin();
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
            return None;
        }
        let plugin = self.plugins.remove(index);
        self.find_world_plugin();
        Some(plugin)
    }

    pub fn set_world_plugin(&mut self, plugin: Plugin) {
        if let Some(world_plugin) = self
            .plugins
            .iter_mut()
            .find(|plugin| plugin.metadata.is_world_plugin)
        {
            world_plugin.metadata = plugin.metadata;
            world_plugin.disabled = plugin.disabled;
            world_plugin.script = plugin.script;
        } else {
            self.plugins.push(plugin);
        }
        self.plugins.sort_unstable();
        self.find_world_plugin();
    }

    pub fn world_plugin(&self) -> Option<&Plugin> {
        self.plugins.get(self.world_script_index?)
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
