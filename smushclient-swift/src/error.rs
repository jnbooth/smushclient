use std::error::Error;
use std::fmt::{self, Display, Formatter};

pub trait StringifyResultError<T> {
    fn str(self) -> Result<T, String>;
}

impl<T, E: Display> StringifyResultError<T> for Result<T, E> {
    fn str(self) -> Result<T, String> {
        self.map_err(|e| e.to_string())
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct UnsupportedError(pub &'static str);

impl Display for UnsupportedError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        f.write_str("Unsupported: ")?;
        f.write_str(self.0)
    }
}

impl Error for UnsupportedError {}
