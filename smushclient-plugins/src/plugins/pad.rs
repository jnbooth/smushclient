use std::borrow::Cow;

use crate::send::Occurrence;
use crate::send::{Alias, Timer, Trigger};

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Pad<'a> {
    Script(&'a str),
    Alias {
        plugin: &'a str,
        label: &'a str,
    },
    Timer {
        plugin: &'a str,
        occurrence: Occurrence,
    },
    Trigger {
        plugin: &'a str,
        label: &'a str,
    },
    PacketDebug,
}

impl<'a> Pad<'a> {
    pub fn title(&self) -> Cow<'a, str> {
        match self {
            Self::Script(s) => (*s).into(),
            Self::Alias { plugin, label } => format!("Alias: {label} ({plugin})").into(),
            Self::Timer { plugin, occurrence } => format!("Timer: {occurrence} ({plugin})").into(),
            Self::Trigger { plugin, label } => format!("Trigger: {label} ({plugin})").into(),
            Self::PacketDebug => "Packet debug".into(),
        }
    }

    pub fn new<T: PadSource>(source: &'a T, plugin_name: &'a str) -> Self {
        source.to_pad(plugin_name)
    }
}

pub trait PadSource {
    fn to_pad<'a>(&'a self, plugin_name: &'a str) -> Pad<'a>;
}

impl PadSource for Alias {
    fn to_pad<'a>(&'a self, plugin_name: &'a str) -> Pad<'a> {
        Pad::Alias {
            plugin: plugin_name,
            label: if self.label.is_empty() {
                &self.pattern
            } else {
                &self.label
            },
        }
    }
}

impl PadSource for Timer {
    fn to_pad<'a>(&'a self, plugin_name: &'a str) -> Pad<'a> {
        Pad::Timer {
            plugin: plugin_name,
            occurrence: self.occurrence,
        }
    }
}

impl PadSource for Trigger {
    fn to_pad<'a>(&'a self, plugin_name: &'a str) -> Pad<'a> {
        Pad::Trigger {
            plugin: plugin_name,
            label: if self.label.is_empty() {
                &self.pattern
            } else {
                &self.label
            },
        }
    }
}
