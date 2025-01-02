use crate::convert::Convert;
use cxx_qt_lib::{QByteArray, QColor, QString, QVector};
use mud_transformer::mxp::RgbColor;
use mud_transformer::naws;
use smushclient_plugins::Reaction;

pub fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.to_vec().convert()
}

fn encode_naws(width: u16, height: u16) -> QByteArray {
    QByteArray::from(naws(width, height).as_slice())
}

fn make_regex_from_wildcards(pattern: &QString) -> QString {
    let mut buf = String::new();
    Reaction::make_regex_pattern(&String::from(pattern), &mut buf);
    QString::from(&buf)
}

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qbytearray.h");
        type QByteArray = cxx_qt_lib::QByteArray;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "C++" {
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
    }
}
