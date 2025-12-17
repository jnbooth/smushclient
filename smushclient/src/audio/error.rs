use std::error::Error;
use std::fmt;
use std::io;

pub use rodio::StreamError;
use rodio::decoder::DecoderError;

#[derive(Debug)]
pub enum AudioError {
    DecoderError(DecoderError),
    FileError(io::Error),
    SinkOutOfRange,
}

impl fmt::Display for AudioError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::DecoderError(e) => e.fmt(f),
            Self::FileError(e) => e.fmt(f),
            Self::SinkOutOfRange => f.write_str("expected a sink number between 0 and 10"),
        }
    }
}

impl Error for AudioError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::DecoderError(e) => Some(e),
            Self::FileError(e) => Some(e),
            Self::SinkOutOfRange => None,
        }
    }
}

impl From<io::Error> for AudioError {
    fn from(value: io::Error) -> Self {
        Self::FileError(value)
    }
}

impl From<DecoderError> for AudioError {
    fn from(value: DecoderError) -> Self {
        Self::DecoderError(value)
    }
}
