use enumeration::Enum;
use serde::{Deserialize, Serialize};

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize, Enum,
)]
pub enum SendTarget {
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

    pub const fn is_script(self) -> bool {
        matches!(self, Self::Script | Self::ScriptAfterOmit)
    }
}

pub mod sendto_serde {
    use enumeration::Enum;
    use serde::de::{Error as _, Unexpected};
    use serde::{Deserialize, Deserializer, Serialize, Serializer};

    use super::SendTarget;

    const _: [(); 14] = [(); SendTarget::MAX as usize];
    const EXPECTED: &str = "integer between 0 and 14";

    #[allow(clippy::trivially_copy_pass_by_ref)]
    pub fn serialize<S: Serializer>(value: &SendTarget, serializer: S) -> Result<S::Ok, S::Error> {
        (*value as u8).serialize(serializer)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<SendTarget, D::Error> {
        let pos = <u8>::deserialize(deserializer)?;
        SendTarget::from_index(pos as usize).ok_or(D::Error::invalid_value(
            Unexpected::Unsigned(u64::from(pos)),
            &EXPECTED,
        ))
    }
}
