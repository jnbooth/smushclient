use std::cmp::Ordering;
use std::fmt;
use std::hash::{Hash, Hasher};
use std::str::{self, FromStr};

use serde::de::{self, Deserialize, Deserializer, Unexpected, Visitor};
use serde::ser::{Serialize, Serializer};

mod builder;
pub(crate) use builder::RegexBuilder;

mod captures;
pub use captures::{CaptureMatches, Captures, Match};

mod error;
pub use error::RegexError;

/// A wrapper around [`pcre2::bytes::Regex`] providing additional trait implementations.
#[derive(Clone)]
pub struct Regex(pub(super) pcre2::bytes::Regex);

// pub type RegexError = pcre2::Error;

impl Default for Regex {
    fn default() -> Self {
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
        self.as_str().fmt(f)
    }
}
impl fmt::Debug for Regex {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        self.as_str().fmt(f)
    }
}

impl Serialize for Regex {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.collect_str(self.as_str())
    }
}

impl<'de> Deserialize<'de> for Regex {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct RegexVisitor;

        impl Visitor<'_> for RegexVisitor {
            type Value = Regex;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("a valid regular expression string")
            }

            fn visit_str<E: de::Error>(self, s: &str) -> Result<Self::Value, E> {
                Regex::new(s).map_err(|_| E::invalid_value(Unexpected::Str(s), &self))
            }
        }

        deserializer.deserialize_str(RegexVisitor)
    }
}

impl FromStr for Regex {
    type Err = RegexError;

    #[inline]
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Self::new(s)
    }
}

impl Regex {
    /// Compiles a regular expression. Once compiled, it can be used repeatedly
    /// to search, split or replace text in a string.
    ///
    /// If an invalid expression is given, then an error is returned.
    pub fn new(re: &str) -> Result<Self, RegexError> {
        match pcre2::bytes::Regex::new(re) {
            Ok(re) => Ok(Self(re)),
            Err(e) => Err(RegexError {
                inner: e,
                target: re.to_owned(),
            }),
        }
    }

    pub fn as_str(&self) -> &str {
        self.0.as_str()
    }

    pub fn captures_iter<'s>(&self, subject: &'s str) -> CaptureMatches<'_, 's> {
        CaptureMatches {
            subject,
            inner: self.0.captures_iter(subject.as_bytes()),
        }
    }

    pub fn capture_names(&self) -> &[Option<String>] {
        self.0.capture_names()
    }
}
