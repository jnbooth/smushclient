use std::cmp::Ordering;
use std::fmt;
use std::hash::{Hash, Hasher};
use std::ops::{Deref, DerefMut};
use std::str::{self, FromStr};

use serde::de::{Error as _, Unexpected};
use serde::{Deserialize, Deserializer, Serialize, Serializer};

/// A wrapper around [`pcre2::bytes::Regex`] providing additional trait implementations.
#[derive(Clone)]
pub struct Regex(pcre2::bytes::Regex);

pub type RegexError = pcre2::Error;

impl Deref for Regex {
    type Target = pcre2::bytes::Regex;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl DerefMut for Regex {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl Default for Regex {
    fn default() -> Self {
        #[allow(clippy::unwrap_used)]
        Self(pcre2::bytes::Regex::new("^$").unwrap())
    }
}

impl PartialEq for Regex {
    fn eq(&self, other: &Self) -> bool {
        self.as_str() == other.as_str()
    }
}
impl Eq for Regex {}

impl PartialOrd for Regex {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for Regex {
    fn cmp(&self, other: &Self) -> Ordering {
        self.as_str().cmp(other.as_str())
    }
}

impl Hash for Regex {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.as_str().hash(state);
    }
}

impl fmt::Display for Regex {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.pad(self.as_str())
    }
}
impl fmt::Debug for Regex {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.pad(self.as_str())
    }
}

impl Serialize for Regex {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.collect_str(self.as_str())
    }
}

impl<'de> Deserialize<'de> for Regex {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        let s = String::deserialize(deserializer)?;
        Regex::new(&s)
            .map_err(|_| D::Error::invalid_value(Unexpected::Str(&s), &"valid regular expression"))
    }
}

impl FromStr for Regex {
    type Err = RegexError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        pcre2::bytes::Regex::new(s).map(Self)
    }
}

impl Regex {
    /// # Panics
    ///
    /// Panics if `bytes` is not valid UTF-8.
    #[track_caller]
    pub fn expect(bytes: &[u8]) -> &str {
        str::from_utf8(bytes).expect("invalid UTF-8")
    }

    /// Compiles a regular expression. Once compiled, it can be used repeatedly
    /// to search, split or replace text in a string.
    ///
    /// If an invalid expression is given, then an error is returned.
    pub fn new(re: &str) -> Result<Self, RegexError> {
        pcre2::bytes::Regex::new(re).map(Self)
    }
}
