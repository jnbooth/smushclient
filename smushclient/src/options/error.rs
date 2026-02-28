use std::error::Error;
use std::fmt;
use std::num::{ParseIntError, TryFromIntError};
use std::str::Utf8Error;
use std::string::FromUtf8Error;

use smushclient_plugins::{RegexError, SenderAccessError};

macro_rules! impl_range_error {
    ($e:ty,$t:ty) => {
        impl From<$t> for $e {
            fn from(_: $t) -> Self {
                Self::OptionOutOfRange
            }
        }
    };
}

#[derive(Debug)]
pub enum SetOptionError {
    UnknownOption,
    OptionOutOfRange,
    PluginCannotSetOption,
}

impl fmt::Display for SetOptionError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::UnknownOption => f.write_str("unknown option"),
            Self::OptionOutOfRange => f.write_str("option out of range"),
            Self::PluginCannotSetOption => f.write_str("plugin cannot set option"),
        }
    }
}

impl Error for SetOptionError {}

impl_range_error!(SetOptionError, TryFromIntError);

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum OptionError {
    UnknownOption,
    OptionOutOfRange,
    PluginCannotSetOption,
    InvalidObjectLabel,
    AliasCannotBeEmpty,
    TriggerCannotBeEmpty,
    ScriptNameNotLocated,
    BadRegularExpression,
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

impl From<SetOptionError> for OptionError {
    fn from(value: SetOptionError) -> Self {
        match value {
            SetOptionError::UnknownOption => Self::UnknownOption,
            SetOptionError::OptionOutOfRange => Self::OptionOutOfRange,
            SetOptionError::PluginCannotSetOption => Self::PluginCannotSetOption,
        }
    }
}

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

impl_range_error!(OptionError, TryFromIntError);
impl_range_error!(OptionError, ParseIntError);
impl_range_error!(OptionError, FromUtf8Error);
impl_range_error!(OptionError, Utf8Error);
