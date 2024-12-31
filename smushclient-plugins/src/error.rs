use std::error::Error;
use std::fmt::{self, Display, Formatter};
use std::io;
use std::ops::RangeInclusive;

use serde::de::Expected;

#[derive(Debug)]
pub enum LoadError {
    File(io::Error),
    Parse(quick_xml::DeError),
}

impl Display for LoadError {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
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

#[derive(Clone, Debug)]
pub struct ExpectedRange<T>(pub RangeInclusive<T>);

impl<T: Display> Expected for ExpectedRange<T> {
    fn fmt(&self, formatter: &mut Formatter) -> fmt::Result {
        write!(
            formatter,
            "integer between {} and {}",
            self.0.start(),
            self.0.end()
        )
    }
}
