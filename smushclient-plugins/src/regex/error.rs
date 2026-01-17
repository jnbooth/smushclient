use std::error::Error;
use std::fmt;
use std::ops::Deref;

#[derive(Clone, Debug)]
pub struct RegexError {
    pub(super) inner: pcre2::Error,
    pub(super) target: String,
}

impl RegexError {
    pub fn target(&self) -> &str {
        &self.target
    }
}

impl Deref for RegexError {
    type Target = pcre2::Error;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl fmt::Display for RegexError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.inner.fmt(f)
    }
}

impl Error for RegexError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        Some(&self.inner)
    }
}
