use serde::Deserialize;
use smushclient_plugins::XmlVec;

use super::bool_serde;
use crate::client::PluginVariables;
use crate::world::NumpadMapping;

#[derive(Debug, Deserialize)]
#[serde(rename = "include")]
pub(super) struct Include {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@plugin", with = "bool_serde", default)]
    pub plugin: bool,
}

#[derive(Debug, Deserialize)]
#[serde(rename = "variable")]
pub(super) struct Variable {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "$text")]
    pub text: String,
}

impl From<XmlVec<Variable>> for PluginVariables {
    fn from(value: XmlVec<Variable>) -> Self {
        let mut vars = PluginVariables::default();
        for var in value.elements {
            vars.set_variable("", var.name.into_bytes(), var.text.into_bytes());
        }
        vars
    }
}

#[derive(Debug, Deserialize)]
#[serde(rename = "key")]
pub(super) struct Key {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(default)]
    pub send: String,
}

impl From<XmlVec<Key>> for NumpadMapping {
    fn from(keypad: XmlVec<Key>) -> Self {
        if keypad.is_empty() {
            return Self::navigation();
        }
        let mut numpad_shortcuts = Self::default();
        for key in keypad.elements {
            let (keys, code) = match key.name.strip_prefix("Ctrl+") {
                Some(code) => (&mut numpad_shortcuts.modified, code),
                None => (&mut numpad_shortcuts.base, key.name.as_str()),
            };
            match code {
                "0" => keys.key_0 = key.send,
                "1" => keys.key_1 = key.send,
                "2" => keys.key_2 = key.send,
                "3" => keys.key_3 = key.send,
                "4" => keys.key_4 = key.send,
                "5" => keys.key_5 = key.send,
                "6" => keys.key_6 = key.send,
                "7" => keys.key_7 = key.send,
                "8" => keys.key_8 = key.send,
                "9" => keys.key_9 = key.send,
                "." => keys.key_period = key.send,
                "/" => keys.key_slash = key.send,
                "*" => keys.key_asterisk = key.send,
                "-" => keys.key_minus = key.send,
                "+" => keys.key_plus = key.send,
                _ => (),
            }
        }
        numpad_shortcuts
    }
}
