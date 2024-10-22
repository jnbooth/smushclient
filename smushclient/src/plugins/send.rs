use mud_transformer::Output;
use smushclient_plugins::{Pad, PluginIndex, Sender, SenderLockError};
use std::error::Error;
use std::fmt::{self, Display, Formatter};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct SendRequest<'a> {
    pub plugin: PluginIndex,
    pub line: &'a str,
    pub wildcards: Vec<&'a str>,
    pub pad: Option<Pad<'a>>,
    pub output: &'a [Output],
    pub sender: &'a Sender,
    pub text: &'a str,
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
