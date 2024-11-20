use mud_transformer::Output;
use smushclient_plugins::{Captures, PluginIndex, Regex, SendTarget};
use std::error::Error;
use std::fmt::{self, Display, Formatter};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct SendRequest<'a> {
    pub plugin: PluginIndex,
    pub send_to: SendTarget,
    pub text: &'a str,
    pub destination: &'a str,
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

#[derive(Clone, Debug)]
pub enum SenderAccessError {
    LabelConflict(usize),
    NotFound,
    Unchanged,
}

impl From<usize> for SenderAccessError {
    fn from(value: usize) -> Self {
        Self::LabelConflict(value)
    }
}

impl Display for SenderAccessError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Self::LabelConflict(_) => f.write_str("sender name conflict"),
            Self::NotFound => f.write_str("sender not found"),
            Self::Unchanged => f.write_str("attempted to replace with the same value"),
        }
    }
}

impl Error for SenderAccessError {}
