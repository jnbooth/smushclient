use cxx_qt_lib::{QByteArray, QColor, QString, QVector};
use mud_transformer::mxp::RgbColor;
use mud_transformer::naws;
use smushclient::LuaStr;
use smushclient_plugins::{Reaction, Regex};

use crate::convert::Convert;

fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.iter().map(Convert::convert).collect()
}

fn encode_naws(width: u16, height: u16) -> QByteArray {
    QByteArray::from(naws(width, height).as_slice())
}

fn make_regex_from_wildcards(pattern: &LuaStr) -> QString {
    let mut buf = String::new();
    Reaction::make_regex_pattern(&String::from_utf8_lossy(pattern), &mut buf);
    QString::from(&buf)
}

fn validate_regex(pattern: &QString) -> ffi::RegexError {
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

    match Regex::new(&String::from(pattern)) {
        Ok(_) => ffi::RegexError {
            message: QString::default(),
            offset: -1,
        },
        Err(e) => ffi::RegexError {
            message: QString::from(format_error(&mut e.to_string())),
            offset: match e.offset() {
                None => -1,
                Some(offset) => i32::try_from(offset).unwrap_or(i32::MAX),
            },
        },
    }
}

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qbytearray.h");
        type QByteArray = cxx_qt_lib::QByteArray;
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qvector.h");
        type QVector_QColor = cxx_qt_lib::QVector<QColor>;
    }

    struct RegexError {
        message: QString,
        offset: i32,
    }

    #[namespace = "ffi"]
    extern "Rust" {
        fn ansi16() -> QVector_QColor;

        #[cxx_name = "encodeNaws"]
        fn encode_naws(width: u16, height: u16) -> QByteArray;

        #[cxx_name = "makeRegexFromWildcards"]
        fn make_regex_from_wildcards(pattern: &[u8]) -> QString;

        #[cxx_name = "validateRegex"]
        fn validate_regex(pattern: &QString) -> RegexError;
    }
}
