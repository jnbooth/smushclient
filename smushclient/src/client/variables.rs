use std::collections::HashMap;
use std::fmt::{self, Display, Formatter};
use std::io::{Read, Write};
use std::ops::{Deref, DerefMut};

use serde::{Deserialize, Serialize};

use crate::world::PersistError;

const CURRENT_VERSION: u8 = 1;

pub type LuaString = Vec<u8>;
pub type LuaStr = [u8];

#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct PluginVariables(HashMap<String, HashMap<LuaString, LuaString>>);

impl Deref for PluginVariables {
    type Target = HashMap<String, HashMap<LuaString, LuaString>>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for PluginVariables {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl Display for PluginVariables {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        for (id, vars) in &self.0 {
            writeln!(f, "{id}:")?;
            for (key, val) in vars {
                let utf8key = String::from_utf8_lossy(key);
                let utf8val = String::from_utf8_lossy(val);
                writeln!(f, "    {utf8key}: {utf8val}")?;
            }
        }
        Ok(())
    }
}

impl PluginVariables {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn get_variable(&self, plugin_id: &str, key: &LuaStr) -> Option<&LuaStr> {
        Some(self.0.get(plugin_id)?.get(key)?.as_slice())
    }

    pub fn has_variable(&self, plugin_id: &str, key: &LuaStr) -> bool {
        match self.0.get(plugin_id) {
            Some(variables) => variables.contains_key(key),
            None => false,
        }
    }

    pub fn set_variable(&mut self, plugin_id: &str, key: LuaString, value: LuaString) {
        if let Some(variables) = self.0.get_mut(plugin_id) {
            variables.insert(key, value);
            return;
        }
        let mut variables = HashMap::new();
        variables.insert(key, value);
        self.0.insert(plugin_id.to_owned(), variables);
    }

    pub fn unset_variable(&mut self, plugin_id: &str, key: &LuaStr) -> Option<LuaString> {
        self.0.get_mut(plugin_id)?.remove(key)
    }

    pub fn save<W: Write>(&self, mut writer: W) -> Result<(), PersistError> {
        writer.write_all(&[CURRENT_VERSION])?;
        bincode::serialize_into(writer, self)?;
        Ok(())
    }

    pub fn load<R: Read>(mut reader: R) -> Result<Self, PersistError> {
        let mut version_buf = [0; 1];
        reader.read_exact(&mut version_buf)?;
        match version_buf[0] {
            1 => bincode::deserialize_from(reader).map_err(Into::into),
            _ => Err(PersistError::Invalid),
        }
    }
}
