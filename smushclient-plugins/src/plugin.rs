use std::borrow::Cow;
use std::cell::{Cell, Ref};
use std::cmp::Ordering;
use std::collections::HashSet;
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
use crate::error::{LoadError, SenderAccessError};
use crate::send::Sender;
use crate::send::{Alias, Timer, Trigger, XmlAlias, XmlTimer, XmlTrigger};
use crate::sort_on_drop::SortOnDrop;
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

mod private {
    pub trait Sealed {}
}

pub trait PluginItem: AsRef<Sender> + AsMut<Sender> + Eq + Ord + Sized + private::Sealed {
    fn for_plugin(plugin: &Plugin) -> &CursorVec<Self>;

    fn assert_unique_label(&self, senders: &CursorVec<Self>) -> Result<(), usize> {
        let label = self.as_ref().label.as_str();
        if label.is_empty() {
            return Ok(());
        }
        match senders.position(|sender| sender.as_ref().label == label) {
            None => Ok(()),
            Some(pos) => Err(pos),
        }
    }
}

impl private::Sealed for Alias {}
impl PluginItem for Alias {
    fn for_plugin(plugin: &Plugin) -> &CursorVec<Self> {
        &plugin.aliases
    }
}

impl private::Sealed for Timer {}
impl PluginItem for Timer {
    fn for_plugin(plugin: &Plugin) -> &CursorVec<Self> {
        &plugin.timers
    }
}

impl private::Sealed for Trigger {}
impl PluginItem for Trigger {
    fn for_plugin(plugin: &Plugin) -> &CursorVec<Self> {
        &plugin.triggers
    }
}

impl Plugin {
    pub fn senders<T: PluginItem>(&self) -> &CursorVec<T> {
        T::for_plugin(self)
    }

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

    pub fn to_xml<W: Write>(&self, mut writer: W) -> Result<(), SeError> {
        let mut serializer = quick_xml::se::Serializer::new(&mut writer);
        serializer.empty_element_handling(quick_xml::se::EmptyElementHandling::Expanded);
        self.serialize(serializer)?;
        Ok(())
    }

    pub fn to_xml_string(&self) -> Result<String, SeError> {
        let mut buf = String::new();
        self.to_xml(&mut buf)?;
        Ok(buf)
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

    pub fn add_sender<T: PluginItem>(&self, sender: T) -> Result<Ref<'_, T>, usize> {
        let senders = self.senders::<T>();
        sender.assert_unique_label(senders)?;
        Ok(senders.insert(sender))
    }

    pub fn replace_sender<T: PluginItem>(
        &self,
        index: usize,
        sender: T,
    ) -> Result<(usize, Ref<'_, T>), SenderAccessError> {
        let senders = self.senders::<T>();
        if *senders.get(index).ok_or(SenderAccessError::NotFound)? == sender {
            return Err(SenderAccessError::Unchanged);
        }
        if let Err(pos) = sender.assert_unique_label(senders)
            && pos != index
        {
            return Err(pos.into());
        }
        Ok(senders.replace(index, sender))
    }

    pub fn import_senders<T: PluginItem>(&self, imported: &mut Vec<T>) -> SortOnDrop<'_, T> {
        let mut senders = self.senders::<T>().borrow_mut();
        let senders_len = senders.len();
        let need_relabeling = !imported
            .iter()
            .all(|sender| sender.as_ref().label.is_empty());
        senders.append(imported);
        if need_relabeling {
            let mut labels = HashSet::new();
            for sender in senders.iter_mut() {
                let sender = sender.as_mut();
                if !labels.contains(&sender.label) {
                    labels.insert(&sender.label);
                    continue;
                }
                let len = sender.label.len();
                for i in 0.. {
                    write!(sender.label, "{i}").unwrap();
                    if !labels.contains(&sender.label) {
                        labels.insert(&sender.label);
                        break;
                    }
                    sender.label.truncate(len);
                }
            }
        }
        SortOnDrop::new(senders, senders_len)
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
        let metadata = PluginMetadata {
            written: NaiveDate::from_epoch_days(1).unwrap(),
            modified: NaiveDate::from_epoch_days(1).unwrap(),
            ..Default::default()
        };
        let plugin = PluginFile {
            plugin: Cow::Owned(metadata),
            triggers: (&*triggers).into(),
            aliases: XmlVec::default(),
            timers: XmlVec::default(),
            script: Cow::default(),
        };

        let mut xml = String::new();
        let mut serializer = quick_xml::se::Serializer::new(&mut xml);
        serializer.empty_element_handling(quick_xml::se::EmptyElementHandling::Expanded);
        plugin
            .serialize(serializer)
            .expect("error serializing plugin");
        assert_eq!(
            xml,
            r#"<muclient><plugin sequence="0" name="" author="" id="" date_written="1970-01-02" date_modified="1970-01-02"></plugin><triggers muclient_version="" world_file_version="" date_saved=""><trigger enabled="y" sequence="100"></trigger><trigger enabled="y" sequence="100"></trigger></triggers></muclient>"#
        );
    }
}
