use serde::{Deserialize, Serialize};

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub enum SendTarget {
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

impl Default for SendTarget {
    fn default() -> Self {
        Self::World
    }
}

impl SendTarget {
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

    pub const fn is_notepad(self) -> bool {
        matches!(
            self,
            Self::NotepadAppend | Self::NotepadNew | Self::NotepadReplace
        )
    }
}

pub mod sendto_serde {
    use serde::de::{Error as _, Unexpected};
    use serde::{Deserializer, Serializer};

    use super::*;

    pub fn serialize<S: Serializer>(value: &SendTarget, serializer: S) -> Result<S::Ok, S::Error> {
        (*value as u8).serialize(serializer)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<SendTarget, D::Error> {
        let pos = <u8>::deserialize(deserializer)?;
        match pos {
            0 => Ok(SendTarget::World),
            1 => Ok(SendTarget::Command),
            2 => Ok(SendTarget::Output),
            3 => Ok(SendTarget::Status),
            4 => Ok(SendTarget::NotepadNew),
            5 => Ok(SendTarget::NotepadAppend),
            6 => Ok(SendTarget::Log),
            7 => Ok(SendTarget::NotepadReplace),
            8 => Ok(SendTarget::WorldDelay),
            9 => Ok(SendTarget::Variable),
            10 => Ok(SendTarget::Execute),
            11 => Ok(SendTarget::Speedwalk),
            12 => Ok(SendTarget::Script),
            13 => Ok(SendTarget::WorldImmediate),
            14 => Ok(SendTarget::ScriptAfterOmit),
            _ => Err(D::Error::invalid_value(
                Unexpected::Unsigned(pos as u64),
                &"integer between 0 and 14",
            )),
        }
    }
}
