use std::convert::Infallible;
use std::error::Error;
use std::fmt;
use std::io;

use crate::regex::RegexError;

#[derive(Debug)]
pub enum LoadError {
    File(io::Error),
    Parse(quick_xml::DeError),
}

impl fmt::Display for LoadError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            LoadError::File(error) => error.fmt(f),
            LoadError::Parse(error) => error.fmt(f),
        }
    }
}

impl Error for LoadError {}

impl From<io::Error> for LoadError {
    fn from(value: io::Error) -> Self {
        Self::File(value)
    }
}

impl From<io::ErrorKind> for LoadError {
    fn from(value: io::ErrorKind) -> Self {
        Self::File(io::Error::from(value))
    }
}

impl From<quick_xml::DeError> for LoadError {
    fn from(value: quick_xml::DeError) -> Self {
        Self::Parse(value)
    }
}

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

impl From<Infallible> for ImportError {
    fn from(value: Infallible) -> Self {
        match value {}
    }
}
