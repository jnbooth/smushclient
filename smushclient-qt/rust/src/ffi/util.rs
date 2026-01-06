use cxx_qt_lib::{QByteArray, QColor, QString, QVector};
use mud_transformer::mxp::RgbColor;
use mud_transformer::naws;
use smushclient_plugins::{Reaction, Regex};

use crate::convert::Convert;

fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.iter().map(Convert::convert).collect()
}

fn encode_naws(width: u16, height: u16) -> QByteArray {
    QByteArray::from(naws(width, height).as_slice())
}

fn make_regex_from_wildcards(pattern: &QString) -> QString {
    let mut buf = String::new();
    Reaction::make_regex_pattern(&String::from(pattern), &mut buf);
    QString::from(&buf)
}

fn validate_regex(pattern: &QString) -> QString {
    match Regex::new(&String::from(pattern)) {
        Ok(_) => QString::default(),
        Err(e) => QString::from(e.to_string()),
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

    #[namespace = "ffi"]
    extern "Rust" {
        fn ansi16() -> QVector_QColor;

        #[cxx_name = "encodeNaws"]
        fn encode_naws(width: u16, height: u16) -> QByteArray;

        #[cxx_name = "makeRegexFromWildcards"]
        fn make_regex_from_wildcards(pattern: &QString) -> QString;

        #[cxx_name = "validateRegex"]
        fn validate_regex(pattern: &QString) -> QString;
    }
}
