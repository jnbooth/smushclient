use cxx_qt_lib::QString;
use smushclient::LuaStr;
use smushclient_plugins::{Reaction, Regex, RegexError};

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    struct RegexParse {
        success: bool,
        message: QString,
        target: QString,
        offset: i32,
    }

    #[namespace = "ffi::regex"]
    extern "Rust" {
        #[cxx_name = "fromWildcards"]
        fn make_regex_from_wildcards(pattern: &[u8]) -> QString;

        #[cxx_name = "validate"]
        fn validate_regex(pattern: &QString) -> RegexParse;
    }
}

fn make_regex_from_wildcards(pattern: &LuaStr) -> QString {
    let pattern = String::from_utf8_lossy(pattern);
    QString::from(&Reaction::make_regex_pattern(&pattern))
}

impl Default for ffi::RegexParse {
    fn default() -> Self {
        Self {
            success: true,
            message: QString::default(),
            target: QString::default(),
            offset: -1,
        }
    }
}

impl From<RegexError> for ffi::RegexParse {
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
            success: false,
            message: QString::from(format_error(&mut value.to_string())),
            target: QString::from(value.target()),
            offset: match value.offset() {
                None => -1,
                Some(offset) => i32::try_from(offset).unwrap_or(i32::MAX),
            },
        }
    }
}

fn validate_regex(pattern: &QString) -> ffi::RegexParse {
    match Regex::new(&String::from(pattern)) {
        Ok(_) => ffi::RegexParse::default(),
        Err(e) => ffi::RegexParse::from(e),
    }
}
