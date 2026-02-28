use std::array::TryFromSliceError;
use std::error::Error;
use std::fmt;
use std::io;

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
