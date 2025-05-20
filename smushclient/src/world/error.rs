use std::fmt;
use std::io;

#[derive(Debug)]
pub enum PersistError {
    File(io::Error),
    Serial(bincode::Error),
    Invalid,
}

impl fmt::Display for PersistError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::File(error) => error.fmt(f),
            Self::Serial(error) => error.fmt(f),
            Self::Invalid => f.write_str("invalid savefile"),
        }
    }
}

impl From<io::Error> for PersistError {
    fn from(value: io::Error) -> Self {
        if value.kind() == io::ErrorKind::UnexpectedEof {
            Self::Invalid
        } else {
            Self::File(value)
        }
    }
}

impl From<bincode::Error> for PersistError {
    fn from(value: bincode::Error) -> Self {
        Self::Serial(value)
    }
}
