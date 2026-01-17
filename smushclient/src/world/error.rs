use std::array::TryFromSliceError;
use std::error::Error;
use std::fmt;
use std::io;
use std::num::TryFromIntError;
use std::string::FromUtf8Error;

#[derive(Debug)]
pub enum PersistError {
    File(io::Error),
    Serial(postcard::Error),
    Invalid,
    UnsupportedVersion,
}

impl fmt::Display for PersistError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::File(error) => error.fmt(f),
            Self::Serial(error) => error.fmt(f),
            Self::Invalid => f.write_str("invalid savefile"),
            Self::UnsupportedVersion => f.write_str("unsupported world version"),
        }
    }
}

impl Error for PersistError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::File(e) => Some(e),
            Self::Serial(e) => Some(e),
            Self::Invalid | Self::UnsupportedVersion => None,
        }
    }
}

impl From<io::Error> for PersistError {
    fn from(value: io::Error) -> Self {
        Self::File(value)
    }
}

impl From<postcard::Error> for PersistError {
    fn from(value: postcard::Error) -> Self {
        Self::Serial(value)
    }
}

impl From<TryFromSliceError> for PersistError {
    fn from(_: TryFromSliceError) -> Self {
        Self::Invalid
    }
}

#[derive(Debug)]
pub enum SetOptionError {
    UnknownOption,
    OptionOutOfRange,
    PluginCannotSetOption,
    LogError(io::Error),
}

impl fmt::Display for SetOptionError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::UnknownOption => f.write_str("unknown option"),
            Self::OptionOutOfRange => f.write_str("option out of range"),
            Self::PluginCannotSetOption => f.write_str("plugin cannot set option"),
            Self::LogError(e) => e.fmt(f),
        }
    }
}

impl Error for SetOptionError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::LogError(e) => Some(e),
            _ => None,
        }
    }
}

impl From<TryFromIntError> for SetOptionError {
    fn from(_: TryFromIntError) -> Self {
        Self::OptionOutOfRange
    }
}

impl From<FromUtf8Error> for SetOptionError {
    fn from(_: FromUtf8Error) -> Self {
        Self::OptionOutOfRange
    }
}

impl From<io::Error> for SetOptionError {
    fn from(value: io::Error) -> Self {
        Self::LogError(value)
    }
}
