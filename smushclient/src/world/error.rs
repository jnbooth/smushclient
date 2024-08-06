use std::fmt::{self, Display, Formatter};
use std::io;

#[derive(Debug)]
pub enum PersistError {
    File(io::Error),
    Serial(bincode::Error),
    NotSave,
}

impl Display for PersistError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Self::File(error) => error.fmt(f),
            Self::Serial(error) => error.fmt(f),
            Self::NotSave => f.write_str("invalid savefile"),
        }
    }
}

impl From<io::Error> for PersistError {
    fn from(value: io::Error) -> Self {
        Self::File(value)
    }
}

impl From<bincode::Error> for PersistError {
    fn from(value: bincode::Error) -> Self {
        Self::Serial(value)
    }
}
