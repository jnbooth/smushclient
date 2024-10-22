use std::borrow::Cow;
use std::cmp::Ordering;
use std::fmt::Write;
use std::hash::Hash;
use std::io::BufRead;
use std::path::PathBuf;
use std::str;

use chrono::{NaiveDate, NaiveDateTime, Utc};
pub use quick_xml::DeError as PluginLoadError;
use serde::{Deserialize, Serialize};

use crate::in_place::InPlace;
use crate::send::{Alias, AliasXml, Timer, TimerXml, Trigger, TriggerXml};

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(try_from = "PluginFile")]
pub struct Plugin {
    pub metadata: PluginMetadata,
    pub disabled: bool,
    pub triggers: Vec<Trigger>,
    pub aliases: Vec<Alias>,
    pub timers: Vec<Timer>,
    pub script: String,
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

    pub fn to_xml<W: Write>(&self, writer: W) -> Result<(), PluginLoadError> {
        quick_xml::se::to_writer(writer, self)
    }

    pub fn to_xml_string(&self) -> Result<String, PluginLoadError> {
        quick_xml::se::to_string(self)
    }
}

/// Corresponds to a plugin .xml file.
#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(rename = "muclient")]
struct PluginFile<'a> {
    plugin: PluginMetadata,
    #[serde(borrow, default, skip_serializing_if = "Vec::is_empty")]
    triggers: Vec<Triggers<'a>>,
    #[serde(borrow, default, skip_serializing_if = "Vec::is_empty")]
    aliases: Vec<Aliases<'a>>,
    #[serde(borrow, default, skip_serializing_if = "Vec::is_empty")]
    timers: Vec<Timers<'a>>,
    #[serde(borrow, default, skip_serializing_if = "Vec::is_empty")]
    script: Vec<Cow<'a, str>>,
}

impl TryFrom<PluginFile<'_>> for Plugin {
    type Error = crate::regex::RegexError;

    fn try_from(value: PluginFile) -> Result<Self, Self::Error> {
        let mut plugin = Self {
            metadata: value.plugin,
            disabled: false,
            triggers: XmlList::try_collect(value.triggers)?,
            aliases: XmlList::try_collect(value.aliases)?,
            timers: XmlList::collect(value.timers),
            script: value.script.in_place(),
        };
        plugin.aliases.sort_unstable();
        plugin.timers.sort_unstable();
        plugin.triggers.sort_unstable();
        Ok(plugin)
    }
}

impl<'a> From<&'a Plugin> for PluginFile<'a> {
    fn from(value: &'a Plugin) -> Self {
        Self {
            plugin: value.metadata.clone(),
            triggers: vec![XmlList::from_children(&value.triggers)],
            aliases: vec![XmlList::from_children(&value.aliases)],
            timers: vec![XmlList::from_children(&value.timers)],
            script: value.script.in_place(),
        }
    }
}

trait XmlList: Sized {
    type Item;

    fn from_children<'a, T>(children: &'a [T]) -> Self
    where
        Self::Item: From<&'a T>;

    fn into_children(self) -> Vec<Self::Item>;

    fn collect<T: From<Self::Item>>(lists: Vec<Self>) -> Vec<T> {
        lists
            .into_iter()
            .flat_map(XmlList::into_children)
            .map(T::from)
            .collect()
    }

    fn try_collect<T: TryFrom<Self::Item>>(lists: Vec<Self>) -> Result<Vec<T>, T::Error> {
        lists
            .into_iter()
            .flat_map(XmlList::into_children)
            .map(T::try_from)
            .collect()
    }
}

macro_rules! xml_list {
    ($t:ident, $item:tt, $children:literal) => {
        #[derive(Clone, Debug, Default, Deserialize, Serialize)]
        #[serde(default)]
        struct $t<'a> {
            #[serde(rename = "@muclient_version", skip_serializing_if = "Option::is_none")]
            muclient_version: Option<String>,
            #[serde(
                rename = "@world_file_version",
                skip_serializing_if = "Option::is_none"
            )]
            world_file_version: Option<u32>,
            #[serde(rename = "@date_saved", skip_serializing_if = "Option::is_none")]
            date_saved: Option<NaiveDateTime>,
            #[serde(borrow, default, rename = $children)]
            children: Vec<$item<'a>>,
        }
        impl<'a> XmlList for $t<'a> {
            type Item = $item<'a>;

            fn from_children<'b, T>(children: &'b [T]) -> Self
            where
                Self::Item: From<&'b T>,
            {
                Self {
                    muclient_version: None,
                    world_file_version: None,
                    date_saved: None,
                    children: children.iter().map(Self::Item::from).collect(),
                }
            }

            fn into_children(self) -> Vec<Self::Item> {
                self.children
            }
        }
    };
}

xml_list!(Triggers, TriggerXml, "trigger");
xml_list!(Aliases, AliasXml, "alias");
xml_list!(Timers, TimerXml, "timer");

fn today() -> NaiveDate {
    Utc::now().date_naive()
}

/// World plugins.
#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub struct PluginMetadata {
    /// Evaluation order. Lower is sooner.
    ///
    /// Negative sequences are evaluated before the main world triggers/aliases.
    // Note: This is at the top for Ord-deriving purposes.
    #[serde(default, rename = "@sequence")]
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
    fn test_plugin_roundtrip() {
        let metadata = PluginMetadata {
            name: "Test Plugin".to_owned(),
            author: "Test".to_owned(),
            id: "0".to_owned(),
            protocols: vec![201],
            ..Default::default()
        };
        let plugin = Plugin {
            metadata,
            disabled: false,
            triggers: vec![Trigger::default()],
            aliases: vec![Alias::default()],
            timers: vec![Timer::default()],
            script: String::new(),
        };
        let to_file =
            quick_xml::se::to_string(&PluginFile::from(&plugin)).expect("error serializing plugin");
        let from_file: Plugin =
            quick_xml::de::from_str(&to_file).expect("error deserializing plugin");
        assert_eq!(from_file, plugin);
    }
}
