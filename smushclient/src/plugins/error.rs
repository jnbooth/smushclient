use std::error::Error;
use std::fmt::{self, Display, Formatter};
use std::io;
use std::path::PathBuf;

use smushclient_plugins::PluginLoadError;

#[derive(Debug)]
pub enum LoadError {
    File(io::Error),
    Parse(PluginLoadError),
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

impl From<PluginLoadError> for LoadError {
    fn from(value: PluginLoadError) -> Self {
        Self::Parse(value)
    }
}

#[derive(Debug)]
pub struct LoadFailure {
    pub error: LoadError,
    pub path: PathBuf,
}
