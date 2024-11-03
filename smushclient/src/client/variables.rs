use std::borrow::Borrow;
use std::collections::HashMap;
use std::ffi::c_char;
use std::fmt::{self, Display, Formatter};
use std::io::{Read, Write};
use std::mem::ManuallyDrop;
use std::ops::{Deref, DerefMut};
use std::ptr;

use serde::{Deserialize, Serialize};

use crate::world::PersistError;

const CURRENT_VERSION: u8 = 1;

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
#[repr(transparent)]
#[serde(transparent)]
pub struct LuaString {
    inner: Vec<c_char>,
}

pub type LuaStr = [c_char];

impl Borrow<LuaStr> for LuaString {
    fn borrow(&self) -> &LuaStr {
        &self.inner
    }
}

impl Display for LuaString {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        // SAFETY: Lua strings are signed or unsigned depending on the platform.
        let bytes: &[u8] = unsafe { &*(ptr::from_ref(self.inner.as_slice()) as *const [u8]) };
        f.write_str(&String::from_utf8_lossy(bytes))
    }
}

impl From<LuaString> for Vec<c_char> {
    fn from(value: LuaString) -> Self {
        value.inner
    }
}

// Note: both of the below implementations are required because c_char might be i8 or u8.

impl From<Vec<u8>> for LuaString {
    fn from(v: Vec<u8>) -> Self {
        let mut v = ManuallyDrop::new(v);
        let p = v.as_mut_ptr().cast::<c_char>();
        let len = v.len();
        let cap = v.capacity();
        Self {
            // SAFETY: Lossless conversion between Vec<u8> and Vec<i8>.
            inner: unsafe { Vec::from_raw_parts(p, len, cap) },
        }
    }
}

impl From<Vec<i8>> for LuaString {
    fn from(v: Vec<i8>) -> Self {
        let mut v = ManuallyDrop::new(v);
        let p = v.as_mut_ptr().cast::<c_char>();
        let len = v.len();
        let cap = v.capacity();
        Self {
            // SAFETY: Lossless conversion between Vec<i8> and Vec<u8>.
            inner: unsafe { Vec::from_raw_parts(p, len, cap) },
        }
    }
}

impl From<String> for LuaString {
    fn from(value: String) -> Self {
        value.into_bytes().into()
    }
}

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
                writeln!(f, "    {key}: {val}")?;
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
        Some(self.0.get(plugin_id)?.get(key)?.inner.as_slice())
    }

    pub fn set_variable<K, V>(&mut self, plugin_id: &str, key: K, value: V)
    where
        K: Into<LuaString>,
        V: Into<LuaString>,
    {
        let key = key.into();
        let value = value.into();
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
