use mud_transformer::Output;
use smushclient_plugins::{Captures, Pad, PluginIndex, Regex, SendTarget, SenderLockError};
use std::error::Error;
use std::fmt::{self, Display, Formatter};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct SendRequest<'a> {
    pub plugin: PluginIndex,
    pub send_to: SendTarget,
    pub text: &'a str,
    pub pad: Option<Pad<'a>>,
}

pub struct SendScriptRequest<'a> {
    pub plugin: PluginIndex,
    pub script: &'a str,
    pub label: &'a str,
    pub line: &'a str,
    pub regex: &'a Regex,
    pub wildcards: Option<Captures<'a>>,
    pub output: &'a [Output],
}

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum SenderAccessError {
    LabelConflict(usize),
    Locked,
    NotFound,
}

impl Display for SenderAccessError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Self::LabelConflict(_) => f.write_str("sender name conflict"),
            Self::Locked => f.write_str("sender is locked"),
            Self::NotFound => f.write_str("sender not found"),
        }
    }
}

impl Error for SenderAccessError {}

impl From<SenderLockError> for SenderAccessError {
    fn from(_: SenderLockError) -> Self {
        Self::Locked
    }
}
