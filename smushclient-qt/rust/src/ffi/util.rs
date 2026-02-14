use cxx_qt_lib::{QByteArray, QColor, QString, QStringList, QVector};
use mud_transformer::mxp::RgbColor;
use mud_transformer::naws;
use smushclient_qt_lib::QAbstractScrollArea;

use crate::convert::Convert;
use crate::get_info::font_info;

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qbytearray.h");
        type QByteArray = cxx_qt_lib::QByteArray;
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qfont.h");
        type QFont = cxx_qt_lib::QFont;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;
        include!("cxx-qt-lib/qvector.h");
        type QVector_QColor = cxx_qt_lib::QVector<QColor>;

        include!("smushclient-qt-lib/qabstractscrollarea.h");
        type QAbstractScrollArea = smushclient_qt_lib::QAbstractScrollArea;
    }

    #[namespace = "ffi::util"]
    extern "Rust" {
        fn ansi16() -> QVector_QColor;
        fn encode_naws(browser: &QAbstractScrollArea) -> QByteArray;
        fn font_info(font: &QFont, info_type: i64) -> QVariant;
        fn get_alpha_option_list() -> QStringList;
        fn get_option_list() -> QStringList;
    }
}

fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.iter().map(Convert::convert).collect()
}

#[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
fn encode_naws(browser: &QAbstractScrollArea) -> QByteArray {
    const ADVANCE_SAMPLE: &str = "0123456789";
    #[allow(clippy::cast_possible_wrap)]
    const ADVANCE_SAMPLE_LEN: i32 = ADVANCE_SAMPLE.len() as i32;

    let metrics = browser.font_metrics();
    let margins = browser.contents_margins();
    let advance = metrics.horizontal_advance(&QString::from(ADVANCE_SAMPLE));
    let viewport = browser.maximum_viewport_size();
    let client_width = viewport.width() - margins.left() - margins.right();
    let client_height = viewport.height() - margins.top() - margins.bottom();
    let width = client_width * ADVANCE_SAMPLE_LEN / advance;
    let height = client_height / metrics.line_spacing() - 4;

    QByteArray::from(naws(width as u16, height as u16).as_slice())
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
