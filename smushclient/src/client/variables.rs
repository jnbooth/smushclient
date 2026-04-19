use std::borrow::Cow;
use std::collections::HashMap;
use std::fmt;
use std::hash::BuildHasher;
use std::io::{Read, Write};
use std::ops::{Deref, DerefMut};

use serde::{Deserialize, Serialize};
use smushclient_plugins::xml::XmlVec;

use crate::world::PersistError;

const CURRENT_VERSION: u8 = 2;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub(crate) struct PluginVariables {
    live: PluginVariableMap,
    persistent: PluginVariableMap,
}

impl PluginVariables {
    pub fn load<R: Read>(reader: R) -> Result<Self, PersistError> {
        PluginVariableMap::load(reader).map(Into::into)
    }

    pub fn is_dirty(&self) -> bool {
        self.live != self.persistent
    }

    pub fn save_all<W, I>(&mut self, writer: W, iter: I) -> Result<(), PersistError>
    where
        W: Write,
        I: IntoIterator,
        I::Item: Into<String>,
    {
        for plugin_id in iter {
            self.copy_into_persist(plugin_id.into());
        }
        self.persistent.save(writer)
    }

    pub fn save_one<W: Write>(&mut self, writer: W, plugin_id: String) -> Result<(), PersistError> {
        self.copy_into_persist(plugin_id);
        self.persistent.save(writer)
    }

    fn copy_into_persist(&mut self, plugin_id: String) {
        match self.live.0.get(&plugin_id) {
            Some(vars) => self.persistent.0.insert(plugin_id, vars.clone()),
            None => self.persistent.0.remove(&plugin_id),
        };
    }
}

impl From<PluginVariableMap> for PluginVariables {
    fn from(value: PluginVariableMap) -> Self {
        Self {
            live: value.clone(),
            persistent: value,
        }
    }
}

impl Deref for PluginVariables {
    type Target = PluginVariableMap;

    fn deref(&self) -> &Self::Target {
        &self.live
    }
}

impl DerefMut for PluginVariables {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.live
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(transparent)]
pub(crate) struct PluginVariableMap(HashMap<String, HashMap<String, Vec<u8>>>);

impl fmt::Display for PluginVariableMap {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        for (id, vars) in &self.0 {
            writeln!(f, "{id}:")?;
            for (key, val) in vars {
                writeln!(f, "    {key}: {}", String::from_utf8_lossy(val))?;
            }
        }
        Ok(())
    }
}

impl PluginVariableMap {
    pub fn is_empty(&self) -> bool {
        self.0.values().all(HashMap::is_empty)
    }

    pub fn len(&self) -> usize {
        self.0.values().map(HashMap::len).sum()
    }

    pub fn count_variables(&self, plugin_id: &str) -> usize {
        match self.0.get(plugin_id) {
            Some(vars) => vars.len(),
            None => 0,
        }
    }

    pub fn get_variable(&self, plugin_id: &str, key: &str) -> Option<&[u8]> {
        Some(self.0.get(plugin_id)?.get(key)?.as_slice())
    }

    pub fn get_variables(&self, plugin_id: &str) -> Option<&HashMap<String, Vec<u8>>> {
        self.0.get(plugin_id)
    }

    pub fn has_variable(&self, plugin_id: &str, key: &str) -> bool {
        self.0
            .get(plugin_id)
            .is_some_and(|variables| variables.contains_key(key))
    }

    pub fn set_variable(&mut self, plugin_id: &str, key: &str, value: &[u8]) {
        let Some(variables) = self.0.get_mut(plugin_id) else {
            let mut variables = HashMap::new();
            variables.insert(key.to_owned(), value.to_owned());
            self.0.insert(plugin_id.to_owned(), variables);
            return;
        };
        match variables.get_mut(key) {
            Some(entry) => {
                value.clone_into(entry);
            }
            None => {
                variables.insert(key.to_owned(), value.to_owned());
            }
        }
    }

    pub fn unset_variable(&mut self, plugin_id: &str, key: &str) -> Option<Vec<u8>> {
        self.0.get_mut(plugin_id)?.remove(key)
    }

    pub fn export_variable(
        &self,
        plugin_id: &str,
        name: &str,
    ) -> Result<String, quick_xml::SeError> {
        let Some(value) = self.get_variable(plugin_id, name) else {
            return Ok(String::new());
        };
        quick_xml::se::to_string(&XmlVariable {
            name: name.into(),
            value: String::from_utf8_lossy(value),
        })
    }

    pub(crate) fn world_variables_mut(&mut self) -> &mut HashMap<String, Vec<u8>> {
        self.0.entry(String::new()).or_default()
    }

    fn save<W: Write>(&self, mut writer: W) -> Result<(), PersistError> {
        writer.write_all(&[CURRENT_VERSION])?;
        postcard::to_io(self, writer)?;
        Ok(())
    }

    fn load<R: Read>(mut reader: R) -> Result<Self, PersistError> {
        let mut buf = Vec::new();
        reader.read_to_end(&mut buf)?;
        let [version, bytes @ ..] = &*buf else {
            return Err(PersistError::Invalid);
        };
        match version {
            2 => postcard::from_bytes(bytes).map_err(Into::into),
            _ => Err(PersistError::Invalid),
        }
    }
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(rename = "variable")]
pub(crate) struct XmlVariable<'a> {
    #[serde(rename = "@name", borrow)]
    pub name: Cow<'a, str>,
    #[serde(rename = "$text", borrow)]
    pub value: Cow<'a, str>,
}

impl<'a, S: BuildHasher> Extend<XmlVariable<'a>> for HashMap<String, Vec<u8>, S> {
    fn extend<T: IntoIterator<Item = XmlVariable<'a>>>(&mut self, iter: T) {
        self.extend(
            iter.into_iter()
                .map(|var| (var.name.into_owned(), var.value.into_owned().into_bytes())),
        );
    }
}

impl<'a, S: BuildHasher + Default> FromIterator<XmlVariable<'a>> for HashMap<String, Vec<u8>, S> {
    fn from_iter<T: IntoIterator<Item = XmlVariable<'a>>>(iter: T) -> Self {
        let mut vars = Self::default();
        vars.extend(iter);
        vars
    }
}

impl From<XmlVec<XmlVariable<'_>>> for PluginVariableMap {
    fn from(value: XmlVec<XmlVariable>) -> Self {
        let mut vars = PluginVariableMap::default();
        vars.0.insert(String::new(), value.into_iter().collect());
        vars
    }
}
