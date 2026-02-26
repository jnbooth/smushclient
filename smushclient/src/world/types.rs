use std::fs::OpenOptions;

use serde::{Deserialize, Serialize};

pub use super::numpad::NumpadMapping;

#[repr(u8)]
#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize,
)]
pub enum AutoConnect {
    #[default]
    None,
    Mush,
    Diku,
    Mxp,
}

#[repr(u8)]
#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize,
)]
pub enum ScriptRecompile {
    #[default]
    Confirm,
    Always,
    Never,
}

#[repr(u8)]
#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize,
)]
pub enum LogFormat {
    #[default]
    Text,
    Html,
    Raw,
}

#[repr(u8)]
#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize,
)]
pub enum LogMode {
    #[default]
    Append,
    Overwrite,
}

#[repr(u8)]
#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize,
)]
pub enum MxpDebugLevel {
    #[default]
    None,
    Error,
    Warning,
    Info,
    All,
}

impl From<LogMode> for OpenOptions {
    fn from(value: LogMode) -> Self {
        let mut options = Self::new();
        match value {
            LogMode::Append => options.append(true),
            LogMode::Overwrite => options.write(true).truncate(true),
        };
        options.create(true);
        options
    }
}
