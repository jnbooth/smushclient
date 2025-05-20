use std::error::Error;
use std::fmt;

pub trait StringifyResultError<T> {
    fn str(self) -> Result<T, String>;
}

impl<T, E: fmt::Display> StringifyResultError<T> for Result<T, E> {
    fn str(self) -> Result<T, String> {
        self.map_err(|e| e.to_string())
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct UnsupportedError(pub &'static str);

impl fmt::Display for UnsupportedError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Unsupported: {}", self.0)
    }
}

impl Error for UnsupportedError {}
