use enumeration::Enum;
use mud_transformer::mxp::HexColor;
use serde::{Deserialize, Serialize};

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize, Enum,
)]
pub enum ProxyType {
    Socks4,
    Socks5,
}

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize, Enum,
)]
pub enum AutoConnect {
    Mush,
    Diku,
    Mxp,
}

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize, Enum,
)]
pub enum ScriptRecompile {
    Confirm,
    Always,
    Never,
}

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize, Enum,
)]
pub enum LogFormat {
    Text,
    Html,
    Raw,
}

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize, Enum,
)]
pub enum LogMode {
    Append,
    Overwrite,
}

#[derive(
    Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize,
)]
pub struct ColorPair {
    pub foreground: Option<HexColor>,
    pub background: Option<HexColor>,
}
