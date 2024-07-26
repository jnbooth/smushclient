use crate::send::Event;

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Pad {
    Script(String),
    Alias { plugin: String, label: String },
    Timer { plugin: String, event: Event },
    Trigger { plugin: String, label: String },
    PacketDebug,
}

impl Pad {
    pub fn title(&self, pad_name: &str) -> String {
        match self {
            Self::Script(s) => format!("{} - {}", s, pad_name),
            Self::Alias { plugin, label } => {
                format!("Alias: {} ({}) - {}", label, plugin, pad_name)
            }
            Self::Timer { plugin, event } => {
                format!("Timer: {} ({}) - {}", event, plugin, pad_name)
            }
            Self::Trigger { plugin, label } => {
                format!("Trigger: {} ({}) - {}", label, plugin, pad_name)
            }
            Self::PacketDebug => format!("Packet debug - {}", pad_name),
        }
    }
}
