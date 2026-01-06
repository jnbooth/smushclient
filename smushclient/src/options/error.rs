use std::error::Error;
use std::fmt;

use smushclient_plugins::RegexError;

use crate::SenderAccessError;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum OptionError {
    InvalidObjectLabel,
    AliasCannotBeEmpty,
    TriggerCannotBeEmpty,
    ScriptNameNotLocated,
    BadRegularExpression,
    PluginCannotSetOption,
    OptionOutOfRange,
    UnknownOption,
}

impl fmt::Display for OptionError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::InvalidObjectLabel => f.write_str("The name is not valid"),
            Self::AliasCannotBeEmpty | Self::TriggerCannotBeEmpty => {
                f.write_str("The \"match_text\" cannot be empty")
            }
            Self::ScriptNameNotLocated => {
                f.write_str("The script name cannot be located in the script file")
            }
            Self::BadRegularExpression => {
                f.write_str("The regular expression could not be evaluated")
            }
            Self::PluginCannotSetOption => f.write_str("The option is marked as non-settable"),
            Self::OptionOutOfRange => f.write_str("The option value is out of range"),
            Self::UnknownOption => f.write_str("Unknown option name"),
        }
    }
}

impl Error for OptionError {}

impl From<RegexError> for OptionError {
    fn from(_: RegexError) -> Self {
        Self::BadRegularExpression
    }
}

impl From<SenderAccessError> for OptionError {
    fn from(_: SenderAccessError) -> Self {
        Self::InvalidObjectLabel
    }
}
