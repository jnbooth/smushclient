use std::ops::{Deref, DerefMut};

use super::Regex;
use super::error::RegexError;

#[derive(Clone, Debug)]
pub(crate) struct RegexBuilder(pcre2::bytes::RegexBuilder);

impl Default for RegexBuilder {
    fn default() -> Self {
        Self::new()
    }
}

impl RegexBuilder {
    pub fn new() -> Self {
        Self(pcre2::bytes::RegexBuilder::new())
    }

    pub fn build(self, pattern: &str) -> Result<Regex, RegexError> {
        match self.0.build(pattern) {
            Ok(re) => Ok(Regex(re)),
            Err(e) => Err(RegexError {
                inner: e,
                target: pattern.to_owned(),
            }),
        }
    }
}

impl Deref for RegexBuilder {
    type Target = pcre2::bytes::RegexBuilder;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for RegexBuilder {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}
