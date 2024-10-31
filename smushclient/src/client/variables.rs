use std::borrow::Cow;
use std::collections::HashMap;
use std::ffi::c_char;
use std::fmt::{self, Display, Formatter};
use std::io::{Read, Write};
use std::ops::{Deref, DerefMut};
use std::ptr;

use serde::{Deserialize, Serialize};

use crate::world::PersistError;

const CURRENT_VERSION: u8 = 1;

#[derive(Clone, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct PluginVariables(HashMap<String, HashMap<Vec<c_char>, Vec<c_char>>>);

impl Deref for PluginVariables {
    type Target = HashMap<String, HashMap<Vec<c_char>, Vec<c_char>>>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for PluginVariables {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

fn format_slice(val: &[i8]) -> Cow<str> {
    let bytes: &[u8] = unsafe { &*(ptr::from_ref(val) as *const [u8]) };
    String::from_utf8_lossy(bytes)
}

impl Display for PluginVariables {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        for (id, vars) in &self.0 {
            writeln!(f, "{id}:")?;
            for (key, val) in vars {
                writeln!(f, "    {}: {}", format_slice(key), format_slice(val))?;
            }
        }
        Ok(())
    }
}

impl PluginVariables {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn get_variable(&self, plugin_id: &str, key: &[c_char]) -> Option<&Vec<c_char>> {
        self.0.get(plugin_id)?.get(key)
    }

    pub fn set_variable(&mut self, plugin_id: &str, key: Vec<c_char>, value: Vec<c_char>) {
        if let Some(variables) = self.0.get_mut(plugin_id) {
            variables.insert(key, value);
            return;
        }
        let mut variables = HashMap::new();
        variables.insert(key, value);
        self.0.insert(plugin_id.to_owned(), variables);
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
