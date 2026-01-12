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
    use serde::de::{Error as _, Unexpected};
    use serde::{Deserialize, Deserializer, Serialize, Serializer};

    use super::SendTarget;
    use crate::error::ExpectedRange;

    #[allow(clippy::trivially_copy_pass_by_ref)]
    pub fn serialize<S: Serializer>(value: &SendTarget, serializer: S) -> Result<S::Ok, S::Error> {
        (*value as u8).serialize(serializer)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<SendTarget, D::Error> {
        let value = <u8>::deserialize(deserializer)?;
        Ok(match value {
            0 => SendTarget::World,
            1 => SendTarget::Command,
            2 => SendTarget::Output,
            3 => SendTarget::Status,
            4 => SendTarget::NotepadNew,
            5 => SendTarget::NotepadAppend,
            6 => SendTarget::Log,
            7 => SendTarget::NotepadReplace,
            8 => SendTarget::WorldDelay,
            9 => SendTarget::Variable,
            10 => SendTarget::Execute,
            11 => SendTarget::Speedwalk,
            12 => SendTarget::Script,
            13 => SendTarget::WorldImmediate,
            14 => SendTarget::ScriptAfterOmit,
            _ => {
                return Err(D::Error::invalid_value(
                    Unexpected::Unsigned(value.into()),
                    &ExpectedRange(0..=14),
                ));
            }
        })
    }
}
