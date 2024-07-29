use enumeration::Enum;
use serde::{Deserialize, Serialize};

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Enum, Deserialize, Serialize,
)]
pub enum SendTo {
    World,
    WorldDelay,
    WorldImmediate,
    Command,
    Output,
    Status,
    NotepadNew,
    NotepadAppend,
    NotepadReplace,
    Log,
    Speedwalk,
    Execute,
    Variable,
    Script,
    ScriptAfterOmit,
}

impl Default for SendTo {
    fn default() -> Self {
        Self::World
    }
}

impl SendTo {
    pub const fn ignore_empty(self) -> bool {
        !matches!(
            self,
            Self::NotepadAppend
                | Self::NotepadReplace
                | Self::Output
                | Self::Variable
                | Self::NotepadNew
                | Self::Log
        )
    }
}

pub mod sendto_serde {
    use serde::de::{Error as _, Unexpected};
    use serde::{Deserializer, Serializer};

    use super::*;

    pub fn serialize<S: Serializer>(value: &SendTo, serializer: S) -> Result<S::Ok, S::Error> {
        value.index().serialize(serializer)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<SendTo, D::Error> {
        let pos = <usize>::deserialize(deserializer)?;
        <SendTo as Enum>::from_index(pos).ok_or_else(|| {
            D::Error::invalid_value(
                Unexpected::Unsigned(pos as u64),
                &"integer between 0 and 14",
            )
        })
    }
}
