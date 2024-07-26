use std::borrow::Cow;
use std::fmt::Write;
use std::hash::Hash;
use std::io::BufRead;
use std::str;

use chrono::{NaiveDate, NaiveDateTime, Utc};
use quick_xml::DeError;
use serde::{Deserialize, Serialize, Serializer};

use crate::in_place::InPlace;
use crate::send::{Alias, AliasXml, Timer, TimerXml, Trigger, TriggerXml};

#[derive(Clone, PartialEq, Eq, Debug, Deserialize)]
#[serde(try_from = "PluginFile")]
pub struct PluginPack {
    pub metadata: PluginMetadata,
    pub triggers: Vec<Trigger>,
    pub aliases: Vec<Alias>,
    pub timers: Vec<Timer>,
    pub script: String,
}

impl Serialize for PluginPack {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        PluginFile::from(self).serialize(serializer)
    }
}

impl PluginPack {
    pub fn from_xml<R: BufRead>(reader: R) -> Result<Self, DeError> {
        quick_xml::de::from_reader(reader)
    }

    pub fn to_xml<W: Write>(&self, writer: W) -> Result<(), DeError> {
        quick_xml::se::to_writer(writer, self)
    }
}

/// Corresponds to a plugin .xml file.
#[derive(Clone, Debug, Deserialize, Serialize)]
#[serde(rename = "muclient")]
struct PluginFile<'a> {
    plugin: PluginMetadata,
    #[serde(borrow, default)]
    triggers: Vec<Triggers<'a>>,
    #[serde(borrow, default)]
    aliases: Vec<Aliases<'a>>,
    #[serde(borrow, default)]
    timers: Vec<Timers<'a>>,
    #[serde(borrow, default)]
    script: Vec<Cow<'a, str>>,
}

impl TryFrom<PluginFile<'_>> for PluginPack {
    type Error = crate::regex::RegexError;

    fn try_from(value: PluginFile) -> Result<Self, Self::Error> {
        Ok(Self {
            metadata: value.plugin,
            triggers: XmlList::try_collect(value.triggers)?,
            aliases: XmlList::try_collect(value.aliases)?,
            timers: XmlList::collect(value.timers),
            script: value.script.in_place(),
        })
    }
}

impl<'a> From<&'a PluginPack> for PluginFile<'a> {
    fn from(value: &'a PluginPack) -> Self {
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
    /// Plugin name.
    #[serde(rename = "@name")]
    pub name: String,
    /// Who wrote it?
    #[serde(rename = "@author")]
    pub author: String,
    /// Unique ID.
    #[serde(rename = "@id")]
    pub id: String,
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
    /// Minimum qMUSHclient version required.
    #[serde(default, rename = "@requires", skip_serializing_if = "str::is_empty")]
    pub requires: String,
    #[serde(skip)]
    pub is_world_plugin: bool,
}

impl Default for PluginMetadata {
    fn default() -> Self {
        Self {
            name: String::new(),
            author: String::new(),
            purpose: String::new(),
            description: String::new(),
            id: String::new(),
            written: today(),
            modified: today(),
            version: String::new(),
            requires: String::new(),
            sequence: 0,
            is_world_plugin: false,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pluginpack_roundtrip() {
        let metadata = PluginMetadata {
            name: "Test Plugin".to_owned(),
            author: "Test".to_owned(),
            id: "0".to_owned(),
            ..Default::default()
        };
        let pack = PluginPack {
            metadata,
            triggers: vec![Trigger::default()],
            aliases: vec![Alias::default()],
            timers: vec![Timer::default()],
            script: String::new(),
        };
        let to_file = quick_xml::se::to_string(&pack).expect("error serializing plugin");
        println!("{}", to_file);
        let from_file: PluginPack =
            quick_xml::de::from_str(&to_file).expect("error deserializing plugin");
        assert_eq!(from_file, pack);
    }
}