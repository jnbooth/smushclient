use crate::send::Occurrence;

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
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
    pub fn title(&self, pad_name: &str) -> String {
        match self {
            Self::Script(s) => format!("{s} - {pad_name}"),
            Self::Alias { plugin, label } => {
                format!("Alias: {label} ({plugin}) - {pad_name}")
            }
            Self::Timer { plugin, occurrence } => {
                format!("Timer: {occurrence} ({plugin}) - {pad_name}")
            }
            Self::Trigger { plugin, label } => {
                format!("Trigger: {label} ({plugin}) - {pad_name}")
            }
            Self::PacketDebug => format!("Packet debug - {pad_name}"),
        }
    }
}
