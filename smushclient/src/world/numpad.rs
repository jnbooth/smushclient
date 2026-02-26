use std::borrow::Cow;

use serde::{Deserialize, Serialize};
use smushclient_plugins::XmlVec;

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

impl Numpad {
    const fn get(&self, name: u8) -> Option<&str> {
        match name {
            b'0' => Some(self.key_0.as_str()),
            b'1' => Some(self.key_1.as_str()),
            b'2' => Some(self.key_2.as_str()),
            b'3' => Some(self.key_3.as_str()),
            b'4' => Some(self.key_4.as_str()),
            b'5' => Some(self.key_5.as_str()),
            b'6' => Some(self.key_6.as_str()),
            b'7' => Some(self.key_7.as_str()),
            b'8' => Some(self.key_8.as_str()),
            b'9' => Some(self.key_9.as_str()),
            b'.' => Some(self.key_period.as_str()),
            b'/' => Some(self.key_slash.as_str()),
            b'*' => Some(self.key_asterisk.as_str()),
            b'-' => Some(self.key_minus.as_str()),
            b'+' => Some(self.key_plus.as_str()),
            _ => None,
        }
    }

    const fn get_mut(&mut self, name: u8) -> Option<&mut String> {
        match name {
            b'0' => Some(&mut self.key_0),
            b'1' => Some(&mut self.key_1),
            b'2' => Some(&mut self.key_2),
            b'3' => Some(&mut self.key_3),
            b'4' => Some(&mut self.key_4),
            b'5' => Some(&mut self.key_5),
            b'6' => Some(&mut self.key_6),
            b'7' => Some(&mut self.key_7),
            b'8' => Some(&mut self.key_8),
            b'9' => Some(&mut self.key_9),
            b'.' => Some(&mut self.key_period),
            b'/' => Some(&mut self.key_slash),
            b'*' => Some(&mut self.key_asterisk),
            b'-' => Some(&mut self.key_minus),
            b'+' => Some(&mut self.key_plus),
            _ => None,
        }
    }
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

    pub const fn get(&self, name: &str) -> Option<&str> {
        match name.as_bytes() {
            [name] => self.base.get(*name),
            [b'C', b't', b'r', b'l', b'+', name] => self.modified.get(*name),
            _ => None,
        }
    }

    pub const fn get_mut(&mut self, name: &str) -> Option<&mut String> {
        match name.as_bytes() {
            [name] => self.base.get_mut(*name),
            [b'C', b't', b'r', b'l', b'+', name] => self.modified.get_mut(*name),
            _ => None,
        }
    }

    pub fn export_key(&self, name: &str) -> Result<String, quick_xml::SeError> {
        let Some(send) = self.get(name) else {
            return Ok(String::new());
        };
        quick_xml::se::to_string(&XmlKey {
            name: name.into(),
            send: send.into(),
        })
    }
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(rename = "key")]
pub(crate) struct XmlKey<'a> {
    #[serde(rename = "@name", borrow)]
    pub name: Cow<'a, str>,
    #[serde(rename = "$text", default, borrow)]
    pub send: Cow<'a, str>,
}

impl From<XmlVec<XmlKey<'_>>> for NumpadMapping {
    fn from(keypad: XmlVec<XmlKey>) -> Self {
        if keypad.is_empty() {
            return Self::navigation();
        }
        let mut numpad_shortcuts = Self::default();
        for key in keypad.elements {
            if let Some(key_mut) = numpad_shortcuts.get_mut(&key.name) {
                *key_mut = key.send.into_owned();
            }
        }
        numpad_shortcuts
    }
}
