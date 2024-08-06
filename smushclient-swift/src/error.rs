use std::fmt::Display;

pub trait StringifyResultError<T> {
    fn str(self) -> Result<T, String>;
}

impl<T, E: Display> StringifyResultError<T> for Result<T, E> {
    fn str(self) -> Result<T, String> {
        self.map_err(|e| e.to_string())
    }
}
