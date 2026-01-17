use std::error::Error;
use std::{fmt, io};

use smushclient_plugins::RegexError;

#[derive(Debug)]
pub enum ImportError {
    Io(io::Error),
    Regex(RegexError),
    Xml(quick_xml::DeError),
}

impl fmt::Display for ImportError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::Io(error) => error.fmt(f),
            Self::Regex(error) => error.fmt(f),
            Self::Xml(error) => error.fmt(f),
        }
    }
}

impl Error for ImportError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::Io(error) => Some(error),
            Self::Regex(error) => Some(error),
            Self::Xml(error) => Some(error),
        }
    }
}

impl From<io::Error> for ImportError {
    fn from(value: io::Error) -> Self {
        Self::Io(value)
    }
}

impl From<RegexError> for ImportError {
    fn from(value: RegexError) -> Self {
        Self::Regex(value)
    }
}

impl From<quick_xml::DeError> for ImportError {
    fn from(value: quick_xml::DeError) -> Self {
        Self::Xml(value)
    }
}
