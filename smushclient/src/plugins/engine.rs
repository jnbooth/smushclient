use std::collections::HashSet;
use std::io::{self};
use std::ops::{Deref, DerefMut};
use std::path::Path;
use std::{slice, vec};

use smushclient_plugins::{LoadError, Plugin, PluginIndex};

use super::error::LoadFailure;
use crate::world::World;

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct PluginEngine {
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
        let plugin = &mut self.plugins[index];
        let path = plugin.metadata.path.clone();
        plugin.disabled.set(true);
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
