use std::borrow::Cow;
use std::io::{Read, Write};

use serde::{Deserialize, Serialize};
use smushclient_plugins::{Alias, Timer, Trigger};

mod config;
pub use config::WorldConfig;

mod error;
pub use error::{PersistError, SetOptionError};

mod escaping;
pub use escaping::{Escaped, EscapedBrackets, LogBrackets};

mod numpad;
pub(crate) use numpad::XmlKey;
pub use numpad::{Numpad, NumpadMapping};

mod option;
pub use option::OptionCaller;

mod sender_map;
pub use sender_map::SenderMap;

mod serde_helpers;
use serde_helpers::skip_temporary;

mod types;
pub use types::*;

mod versions;

const CURRENT_VERSION: u16 = 6;

#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct World<'a> {
    pub config: Cow<'a, WorldConfig>,
    #[serde(serialize_with = "skip_temporary")]
    pub timers: Cow<'a, [Timer]>,
    #[serde(serialize_with = "skip_temporary")]
    pub triggers: Cow<'a, [Trigger]>,
    #[serde(serialize_with = "skip_temporary")]
    pub aliases: Cow<'a, [Alias]>,
}

impl Default for World<'static> {
    fn default() -> Self {
        Self::new()
    }
}

impl World<'static> {
    pub fn new() -> Self {
        Self {
            config: Cow::Owned(WorldConfig::new()),
            timers: Cow::Owned(Vec::new()),
            triggers: Cow::Owned(Vec::new()),
            aliases: Cow::Owned(Vec::new()),
        }
    }

    pub fn load<R: Read>(mut reader: R) -> Result<Self, PersistError> {
        let mut buf = Vec::new();
        reader.read_to_end(&mut buf)?;
        let (version, bytes) = buf.split_at_checked(2).ok_or(PersistError::Invalid)?;
        let version = u16::from_be_bytes(version.try_into()?);
        match version {
            1 => postcard::from_bytes::<versions::V1>(bytes).map(Into::into),
            2 => postcard::from_bytes::<versions::V2>(bytes).map(Into::into),
            3 => postcard::from_bytes::<versions::V3>(bytes).map(Into::into),
            4 => postcard::from_bytes::<versions::V4>(bytes).map(Into::into),
            5 => postcard::from_bytes::<versions::V5>(bytes).map(Into::into),
            6 => postcard::from_bytes(bytes),
            _ => Err(PersistError::UnsupportedVersion)?,
        }
        .map_err(Into::into)
    }
}

impl World<'_> {
    pub fn save<W: Write>(&self, mut writer: W) -> Result<(), PersistError> {
        writer.write_all(&CURRENT_VERSION.to_be_bytes())?;
        postcard::to_io(self, writer)?;
        Ok(())
    }

    /*
    pub fn escape<'a>(&self, message: &'a str) -> Escaped<Cow<'a, str>> {
        Escaped::borrow(message, self)
    }

    pub fn brackets(&self) -> LogBrackets {
        LogBrackets::from(self)
    }

    pub fn log_path(&self) -> String {
        if self.auto_log_file_name.is_empty() {
            let mut path = format!("{} log.txt", self.name);
            path.retain(|c| !"<>\"|?:#%;/\\".contains(c));
            return path;
        }
        let mut path = String::new();
        Local::now()
            .format(&self.auto_log_file_name)
            .write_to(&mut path)
            .unwrap();
        path
    }

    pub fn world_plugin(&self) -> Plugin {
        let today = Local::now().date_naive();
        Plugin {
            disabled: Cell::new(!self.enable_scripts),
            metadata: PluginMetadata {
                name: format!("World Script: {}", self.name),
                written: today,
                modified: today,
                is_world_plugin: true,
                sequence: -1,
                path: PathBuf::from(&self.world_script),
                ..Default::default()
            },
            ..Default::default()
        }
    }

    pub fn add_sender<T: SendIterable>(&self, sender: T) -> Result<Ref<'_, T>, SenderAccessError> {
        let senders = T::from_world(self);
        sender.assert_unique_label(senders)?;
        Ok(senders.insert(sender))
    }

    pub fn replace_sender<T: SendIterable>(
        &self,
        index: usize,
        sender: T,
    ) -> Result<(usize, Ref<'_, T>), SenderAccessError> {
        let senders = T::from_world(self);
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

    pub fn remove_temporary(&self) {
        self.aliases.retain(|sender| !sender.temporary);
        self.timers.retain(|sender| !sender.temporary);
        self.triggers.retain(|sender| !sender.temporary);
    }

    pub fn connect_message(&self) -> String {
        let player = &self.player;
        let password = &self.password;
        let text = if self.connect_text.is_empty() {
            String::new()
        } else {
            let mut text = self
                .connect_text
                .replace("%name%", player)
                .replace("%password%", password);
            if !text.ends_with('\n') {
                text.push_str("\r\n");
            }
            text
        };
        match self.connect_method {
            AutoConnect::Diku if password.is_empty() => format!("{player}\r\n{text}"),
            AutoConnect::Diku => format!("{player}\r\n{password}\r\n{text}"),
            AutoConnect::Mush => format!("connect {player} {password}\r\n{text}"),
            AutoConnect::Mxp | AutoConnect::None => text,
        }
    }

    pub fn import_senders<T: SendIterable>(&self, imported: &mut Vec<T>) -> SortOnDrop<'_, T> {
        let mut senders = T::from_world(self).borrow_mut();
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

impl From<&World> for TransformerConfig {
    fn from(value: &World) -> Self {
        let (player, password) = if value.connect_method == AutoConnect::Mxp {
            (value.player.clone(), value.password.clone())
        } else {
            (String::new(), String::new())
        };
        Self {
            use_mxp: value.use_mxp,
            disable_compression: value.disable_compression,
            terminal_identification: value.terminal_identification.clone(),
            player,
            password,
            convert_ga_to_newline: value.convert_ga_to_newline,
            no_echo_off: value.no_echo_off,
            naws: value.naws,
            disable_utf8: !value.utf_8,
            ignore_mxp_colors: value.ignore_mxp_colour_changes,
            colors: value.ansi_colours.to_vec(),
            ..Default::default()
        }
    }*/
}
