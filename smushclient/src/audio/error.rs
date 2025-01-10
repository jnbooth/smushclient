use std::error::Error;
use std::fmt::{self, Display, Formatter};
use std::io;

use rodio::decoder::DecoderError;
use rodio::{PlayError, StreamError};

#[derive(Debug)]
pub enum AudioError {
    DecoderError(DecoderError),
    FileError(io::Error),
    PlayError(PlayError),
    SinkOutOfRange,
    StreamError(StreamError),
}

impl Display for AudioError {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        match self {
            Self::DecoderError(e) => e.fmt(f),
            Self::FileError(e) => e.fmt(f),
            Self::PlayError(e) => e.fmt(f),
            Self::SinkOutOfRange => f.write_str("expected a sink number between 0 and 10"),
            Self::StreamError(e) => e.fmt(f),
        }
    }
}

impl Error for AudioError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::DecoderError(e) => Some(e),
            Self::FileError(e) => Some(e),
            Self::PlayError(e) => Some(e),
            Self::SinkOutOfRange => None,
            Self::StreamError(e) => Some(e),
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

impl From<PlayError> for AudioError {
    fn from(value: PlayError) -> Self {
        Self::PlayError(value)
    }
}

impl From<StreamError> for AudioError {
    fn from(value: StreamError) -> Self {
        Self::StreamError(value)
    }
}
