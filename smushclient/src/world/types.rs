use std::fs::OpenOptions;

use serde::{Deserialize, Serialize};

#[repr(u8)]
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub enum AutoConnect {
    None,
    Mush,
    Diku,
    Mxp,
}

#[repr(u8)]
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub enum ScriptRecompile {
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
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub enum LogMode {
    Append,
    Overwrite,
}

#[repr(u8)]
#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub enum MXPDebugLevel {
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

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub struct Numpad {
    pub key_0: String,
    pub key_1: String,
    pub key_2: String,
    pub key_3: String,
    pub key_4: String,
    pub key_5: String,
    pub key_6: String,
    pub key_7: String,
    pub key_8: String,
    pub key_9: String,
    pub key_period: String,
    pub key_slash: String,
    pub key_asterisk: String,
    pub key_minus: String,
    pub key_plus: String,
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub struct NumpadMapping {
    pub base: Numpad,
    pub modified: Numpad,
}

impl NumpadMapping {
    pub fn navigation() -> Self {
        Self {
            base: Numpad {
                key_0: "look".to_owned(),
                key_1: "sw".to_owned(),
                key_2: "south".to_owned(),
                key_3: "se".to_owned(),
                key_4: "west".to_owned(),
                key_5: "WHO".to_owned(),
                key_6: "east".to_owned(),
                key_7: "nw".to_owned(),
                key_8: "north".to_owned(),
                key_9: "ne".to_owned(),
                key_period: "hide".to_owned(),
                key_slash: "inventory".to_owned(),
                key_asterisk: "score".to_owned(),
                key_minus: "up".to_owned(),
                key_plus: "down".to_owned(),
            },
            modified: Numpad::default(),
        }
    }
}
