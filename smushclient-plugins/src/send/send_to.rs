use serde::{Deserialize, Serialize};

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
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

const SENDTARGET_MAX: SendTarget = SendTarget::ScriptAfterOmit;

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
    use std::ops::RangeInclusive;

    use serde::de::{Error as _, Unexpected};
    use serde::{Deserialize, Deserializer, Serialize, Serializer};

    use super::SENDTARGET_MAX;
    use super::SendTarget;
    use crate::error::ExpectedRange;

    #[allow(clippy::trivially_copy_pass_by_ref)]
    pub fn serialize<S: Serializer>(value: &SendTarget, serializer: S) -> Result<S::Ok, S::Error> {
        (*value as u8).serialize(serializer)
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<SendTarget, D::Error> {
        const RANGE: RangeInclusive<u8> = 0..=SENDTARGET_MAX as u8;
        let value = <u8>::deserialize(deserializer)?;
        if RANGE.contains(&value) {
            // SAFETY: Since `value` is contained in `RANGE`, it must be a valid value.
            Ok(unsafe { std::mem::transmute::<u8, SendTarget>(value) })
        } else {
            Err(D::Error::invalid_value(
                Unexpected::Unsigned(u64::from(value)),
                &ExpectedRange(RANGE),
            ))
        }
    }
}
