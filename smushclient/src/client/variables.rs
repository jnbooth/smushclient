use std::borrow::Cow;
use std::collections::HashMap;
use std::fmt;
use std::io::{Read, Write};

use serde::{Deserialize, Serialize};
use smushclient_plugins::XmlVec;

use crate::world::PersistError;

const CURRENT_VERSION: u8 = 2;

pub type LuaString = Vec<u8>;
pub type LuaStr = [u8];

#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub(crate) struct PluginVariables(HashMap<String, HashMap<LuaString, LuaString>>);

impl fmt::Display for PluginVariables {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
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

    pub fn get_variable(&self, plugin_id: &str, key: &LuaStr) -> Option<&LuaStr> {
        Some(self.0.get(plugin_id)?.get(key)?.as_slice())
    }

    pub fn has_variable(&self, plugin_id: &str, key: &LuaStr) -> bool {
        self.0
            .get(plugin_id)
            .is_some_and(|variables| variables.contains_key(key))
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
        postcard::to_io(self, writer)?;
        Ok(())
    }

    pub fn load<R: Read>(mut reader: R) -> Result<Self, PersistError> {
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

    pub fn export_variable(
        &self,
        plugin_id: &str,
        name: &str,
    ) -> Result<String, quick_xml::SeError> {
        let Some(value) = self.get_variable(plugin_id, name.as_bytes()) else {
            return Ok(String::new());
        };
        quick_xml::se::to_string(&XmlVariable {
            name: name.into(),
            value: String::from_utf8_lossy(value),
        })
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

impl From<XmlVec<XmlVariable<'_>>> for PluginVariables {
    fn from(value: XmlVec<XmlVariable>) -> Self {
        let worldvars = value
            .elements
            .into_iter()
            .map(|var| {
                (
                    var.name.into_owned().into_bytes(),
                    var.value.into_owned().into_bytes(),
                )
            })
            .collect();
        let mut vars = PluginVariables::default();
        vars.0.insert(String::new(), worldvars);
        vars
    }
}
