use cxx_qt_lib::{QByteArray, QColor, QString, QStringList, QVector};
use mud_transformer::mxp::RgbColor;
use mud_transformer::naws;

use crate::convert::Convert;

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qbytearray.h");
        type QByteArray = cxx_qt_lib::QByteArray;
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
        include!("cxx-qt-lib/qvector.h");
        type QVector_QColor = cxx_qt_lib::QVector<QColor>;
    }

    #[namespace = "ffi"]
    extern "Rust" {
        fn ansi16() -> QVector_QColor;

        #[cxx_name = "encodeNaws"]
        fn encode_naws(width: u16, height: u16) -> QByteArray;

        #[cxx_name = "getAlphaOptionList"]
        fn get_alpha_option_list() -> QStringList;

        #[cxx_name = "getOptionList"]
        fn get_option_list() -> QStringList;
    }
}

fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.iter().map(Convert::convert).collect()
}

fn encode_naws(width: u16, height: u16) -> QByteArray {
    QByteArray::from(naws(width, height).as_slice())
}

fn get_alpha_option_list() -> QStringList {
    let mut list = QStringList::default();
    list.extend(
        smushclient::World::STR_OPTIONS
            .iter()
            .map(|&s| QString::from(s)),
    );
    list
}

fn get_option_list() -> QStringList {
    let mut list = QStringList::default();
    list.extend(
        smushclient::World::INT_OPTIONS
            .iter()
            .map(|&s| QString::from(s)),
    );
    list
}
