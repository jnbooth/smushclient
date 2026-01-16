use serde::{Deserialize, Serialize};

#[repr(u8)]
#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize,
)]
pub enum SendTarget {
    #[default]
    World,
    Command,
    Output,
    Status,
    NotepadNew,
    NotepadAppend,
    Log,
    NotepadReplace,
    WorldDelay,
    Variable,
    Execute,
    Speedwalk,
    Script,
    WorldImmediate,
    ScriptAfterOmit,
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

    pub const fn is_script(self) -> bool {
        matches!(self, Self::Script | Self::ScriptAfterOmit)
    }
}

pub mod sendto_serde {
    use serde::de::{self, Deserialize, Deserializer, Unexpected};
    use serde::ser::{Serialize, Serializer};

    use super::SendTarget;

    #[allow(clippy::trivially_copy_pass_by_ref)]
    pub fn serialize<S: Serializer>(value: &SendTarget, serializer: S) -> Result<S::Ok, S::Error> {
        (*value as u8).serialize(serializer)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<SendTarget, D::Error> {
        let value = <u8>::deserialize(deserializer)?;
        match value {
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
            _ => Err(de::Error::invalid_value(
                Unexpected::Unsigned(value.into()),
                &"an integer between 0 and 14",
            )),
        }
    }
}
