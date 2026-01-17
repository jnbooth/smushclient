use std::borrow::Cow;
use std::cell::Cell;
use std::cmp::Ordering;
use std::fmt::Write;
use std::fs::File;
use std::hash::Hash;
use std::io::{BufRead, BufReader};
use std::path::{Path, PathBuf};
use std::str;

use chrono::{Local, NaiveDate};
pub use quick_xml::DeError as PluginLoadError;
use quick_xml::SeError;
use serde::{Deserialize, Serialize, Serializer};

use crate::cursor_vec::CursorVec;
use crate::error::LoadError;
use crate::send::{Alias, Timer, Trigger, XmlAlias, XmlTimer, XmlTrigger};
use crate::xml::XmlVec;

pub type PluginIndex = usize;

#[derive(Clone, Debug, Default, Deserialize)]
#[serde(try_from = "PluginFile")]
pub struct Plugin {
    pub metadata: PluginMetadata,
    pub disabled: Cell<bool>,
    pub triggers: CursorVec<Trigger>,
    pub aliases: CursorVec<Alias>,
    pub timers: CursorVec<Timer>,
    pub script: String,
}

impl Serialize for Plugin {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let triggers = self.triggers.borrow();
        let aliases = self.aliases.borrow();
        let timers = self.timers.borrow();
        PluginFile {
            plugin: Cow::Borrowed(&self.metadata),
            triggers: (&**triggers).into(),
            aliases: (&**aliases).into(),
            timers: (&**timers).into(),
            script: (&self.script).into(),
        }
        .serialize(serializer)
    }
}

impl PartialEq for Plugin {
    fn eq(&self, other: &Self) -> bool {
        self.metadata.eq(&other.metadata)
    }
}

impl Eq for Plugin {}

impl PartialOrd for Plugin {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Plugin {
    fn cmp(&self, other: &Self) -> Ordering {
        self.metadata.cmp(&other.metadata)
    }
}

impl Plugin {
    pub fn is_empty(&self) -> bool {
        self.triggers.is_empty()
            && self.aliases.is_empty()
            && self.timers.is_empty()
            && self.script.trim().is_empty()
    }

    pub fn from_xml<R: BufRead>(reader: R) -> Result<Self, PluginLoadError> {
        quick_xml::de::from_reader(reader)
    }

    pub fn from_xml_str(s: &str) -> Result<Self, PluginLoadError> {
        quick_xml::de::from_str(s)
    }

    pub fn to_xml<W: Write>(&self, writer: W) -> Result<(), SeError> {
        quick_xml::se::to_writer(writer, self)?;
        Ok(())
    }

    pub fn to_xml_string(&self) -> Result<String, SeError> {
        quick_xml::se::to_string(self)
    }

    pub fn load<P: AsRef<Path>>(path: P) -> Result<Self, LoadError> {
        let path = path.as_ref();
        let file = File::open(path)?;
        let reader = BufReader::new(file);
        let mut this = Self::from_xml(reader)?;
        this.metadata.path = path.to_path_buf();
        Ok(this)
    }

    pub fn remove_temporary(&self) {
        self.aliases.retain(|sender| !sender.temporary);
        self.timers.retain(|sender| !sender.temporary);
        self.triggers.retain(|sender| !sender.temporary);
    }
}

/// Corresponds to a plugin .xml file.
#[derive(Debug, Default, Deserialize, Serialize)]
#[serde(rename = "muclient", default)]
struct PluginFile<'a> {
    plugin: Cow<'a, PluginMetadata>,
    #[serde(borrow, skip_serializing_if = "XmlVec::is_empty")]
    triggers: XmlVec<XmlTrigger<'a>>,
    #[serde(borrow, skip_serializing_if = "XmlVec::is_empty")]
    aliases: XmlVec<XmlAlias<'a>>,
    #[serde(borrow, skip_serializing_if = "XmlVec::is_empty")]
    timers: XmlVec<XmlTimer<'a>>,
    #[serde(borrow, skip_serializing_if = "str::is_empty")]
    script: Cow<'a, str>,
}

impl TryFrom<PluginFile<'_>> for Plugin {
    type Error = crate::regex::RegexError;

    fn try_from(value: PluginFile) -> Result<Self, Self::Error> {
        Ok(Self {
            metadata: value.plugin.into_owned(),
            disabled: Cell::new(false),
            triggers: value.triggers.try_into()?,
            aliases: value.aliases.try_into()?,
            timers: value.timers.try_into().unwrap(),
            script: value.script.into(),
        })
    }
}

fn today() -> NaiveDate {
    Local::now().date_naive()
}

/// World plugins.
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub struct PluginMetadata {
    /// Evaluation order. Lower is sooner.
    ///
    /// Negative sequences are evaluated before the main world triggers/aliases.
    // Note: This is at the top for Ord-deriving purposes.
    #[serde(rename = "@sequence", default)]
    pub sequence: i16,
    // Note: This is also at the top for Ord-deriving purposes.
    #[serde(skip)]
    pub is_world_plugin: bool,
    /// Plugin name.
    #[serde(rename = "@name")]
    pub name: String,
    /// Who wrote it?
    #[serde(rename = "@author")]
    pub author: String,
    /// Unique ID.
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(skip)]
    pub path: PathBuf,
    /// Short description of the plugin's functionality.
    #[serde(default, rename = "@purpose", skip_serializing_if = "str::is_empty")]
    pub purpose: String,
    /// Long description of the plugin's functionality.
    #[serde(
        default,
        rename = "@description",
        skip_serializing_if = "str::is_empty"
    )]
    pub description: String,
    /// Date written.
    #[serde(default = "today", rename = "@date_written")]
    pub written: NaiveDate,
    /// Date last modified.
    #[serde(default = "today", rename = "@date_modified")]
    pub modified: NaiveDate,
    /// Plugin version.
    #[serde(default, rename = "@version", skip_serializing_if = "str::is_empty")]
    pub version: String,
    /// Minimum client version required.
    #[serde(default, rename = "@requires", skip_serializing_if = "str::is_empty")]
    pub requires: String,
    /// Telnet protocols that should receive a WILL response.
    #[serde(default, rename = "@protocols", skip_serializing_if = "Vec::is_empty")]
    pub protocols: Vec<u8>,
}

impl Default for PluginMetadata {
    fn default() -> Self {
        Self {
            name: String::new(),
            author: String::new(),
            purpose: String::new(),
            description: String::new(),
            id: String::new(),
            path: PathBuf::new(),
            written: today(),
            modified: today(),
            version: String::new(),
            requires: String::new(),
            sequence: 0,
            protocols: Vec::new(),
            is_world_plugin: false,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xml_roundtrip() {
        let triggers = vec![Trigger::default(), Trigger::default()];
        let plugin = PluginFile {
            plugin: Cow::Owned(PluginMetadata::default()),
            triggers: (&*triggers).into(),
            aliases: XmlVec::default(),
            timers: XmlVec::default(),
            script: Cow::default(),
        };

        let xml = quick_xml::se::to_string(&plugin).unwrap();
        panic!("{xml}");
    }
}
