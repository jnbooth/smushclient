use std::error::Error;
use std::io;
use std::str::Utf8Error;

use cxx_qt_lib::QString;
use smushclient_plugins::xml::XmlError;
use smushclient_plugins::{ImportError, Reaction, Regex, RegexError};

use crate::ffi::StringView;

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    #[namespace = "rust"]
    extern "C++" {
        include!("smushclient_qt/views.h");
        #[cxx_name = "string_view"]
        type StringView<'a> = crate::ffi::StringView<'a>;
    }

    struct ParseResult {
        code: i64,
        message: QString,
        target: QString,
        offset: i32,
    }

    #[namespace = "ffi::regex"]
    extern "Rust" {
        fn from_wildcards(pattern: StringView) -> QString;
        fn validate(pattern: &QString) -> ParseResult;
    }
}

fn from_wildcards(pattern: StringView) -> QString {
    QString::from(&Reaction::make_regex_pattern(&pattern.to_string_lossy()))
}

impl ffi::ParseResult {
    fn new<E: Error>(error: &E) -> Self {
        Self {
            code: -1,
            message: QString::from(&error.to_string()),
            ..Default::default()
        }
    }
}

impl Default for ffi::ParseResult {
    fn default() -> Self {
        Self {
            code: 0,
            message: QString::default(),
            target: QString::default(),
            offset: -1,
        }
    }
}

impl From<i64> for ffi::ParseResult {
    fn from(code: i64) -> Self {
        Self {
            code,
            ..Default::default()
        }
    }
}

impl From<i32> for ffi::ParseResult {
    fn from(value: i32) -> Self {
        Self::from(i64::from(value))
    }
}

impl From<usize> for ffi::ParseResult {
    fn from(value: usize) -> Self {
        Self::from(value.try_into().unwrap_or(i64::MAX))
    }
}

impl From<Utf8Error> for ffi::ParseResult {
    fn from(value: Utf8Error) -> Self {
        Self {
            offset: value.valid_up_to().try_into().unwrap_or(-1),
            ..Self::new(&value)
        }
    }
}

impl From<io::Error> for ffi::ParseResult {
    fn from(value: io::Error) -> Self {
        Self::new(&value)
    }
}

impl From<XmlError> for ffi::ParseResult {
    fn from(value: XmlError) -> Self {
        Self::new(&value)
    }
}

impl From<RegexError> for ffi::ParseResult {
    fn from(value: RegexError) -> Self {
        fn find_split(message: &str) -> Option<usize> {
            const SKIP: usize = "PCRE2: ".len();
            const DELIM: &str = ": ";
            const OFFSET: usize = SKIP + DELIM.len();
            let message = message.get(SKIP..)?;
            let index = message.find(DELIM)?;
            Some(OFFSET + index)
        }

        fn format_error(mut message: &mut str) -> &str {
            let Some(split) = find_split(message) else {
                return message;
            };
            message = &mut message[split..];
            if let Some(first) = message.get_mut(0..1) {
                first.make_ascii_uppercase();
            }
            message
        }

        Self {
            code: -2,
            message: QString::from(format_error(&mut value.to_string())),
            target: QString::from(value.target()),
            offset: match value.offset() {
                Some(offset) => offset.try_into().unwrap_or(i32::MAX),
                None => -1,
            },
        }
    }
}

impl From<ImportError> for ffi::ParseResult {
    fn from(value: ImportError) -> Self {
        match value {
            ImportError::Io(error) => error.into(),
            ImportError::Regex(error) => error.into(),
            ImportError::Xml(error) => error.into(),
        }
    }
}

impl<T, E> From<Result<T, E>> for ffi::ParseResult
where
    ffi::ParseResult: From<T>,
    ffi::ParseResult: From<E>,
{
    fn from(value: Result<T, E>) -> Self {
        match value {
            Ok(ok) => ok.into(),
            Err(e) => e.into(),
        }
    }
}

fn validate(pattern: &QString) -> ffi::ParseResult {
    match Regex::new(&String::from(pattern)) {
        Ok(_) => ffi::ParseResult::default(),
        Err(e) => ffi::ParseResult::from(e),
    }
}
