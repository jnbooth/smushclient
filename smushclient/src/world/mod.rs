use std::borrow::Cow;
use std::io::{Read, Write};

use serde::{Deserialize, Serialize};
use smushclient_plugins::{Alias, Timer, Trigger};

mod config;
pub use config::WorldConfig;

mod error;
pub use error::{PersistError, SetOptionError};

mod escaping;
pub use escaping::{Escaped, EscapedBrackets, LogBrackets, fixup_html};

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
use versions::Migrate;

const CURRENT_VERSION: u16 = 7;

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
            1 => versions::V1::migrate(bytes),
            2 => versions::V2::migrate(bytes),
            3 => versions::V3::migrate(bytes),
            4 => versions::V4::migrate(bytes),
            5 => versions::V5::migrate(bytes),
            6 => versions::V6::migrate(bytes),
            7 => postcard::from_bytes(bytes),
            _ => return Err(PersistError::UnsupportedVersion),
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
}
