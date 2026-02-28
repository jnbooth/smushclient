use cxx_qt_lib::{QByteArray, QColor, QString, QStringList, QVariant, QVector};
use mud_transformer::mxp::{self, RgbColor};
use mud_transformer::naws;
use smushclient::WorldConfig;
use smushclient_qt_lib::QAbstractScrollArea;

use crate::convert::Convert;
use crate::ffi::{StringView, VariableView};
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

    #[namespace = "rust"]
    extern "C++" {
        include!("smushclient_qt/views.h");
        #[cxx_name = "string_view"]
        type StringView<'a> = crate::ffi::StringView<'a>;
        #[cxx_name = "variable_view"]
        type VariableView = crate::ffi::VariableView;
    }

    #[namespace = "ffi::util"]
    extern "Rust" {
        fn ansi16() -> QVector_QColor;
        fn default_variant_option(option: StringView) -> QVariant;
        fn encode_naws(browser: &QAbstractScrollArea) -> QByteArray;
        fn fixup_html(text: StringView) -> String;
        fn font_info(font: &QFont, info_type: i64) -> QVariant;
        fn get_alpha_option_list() -> QStringList;
        fn get_global_entity(name: StringView) -> VariableView;
        fn get_option_list() -> QStringList;
    }
}

fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.iter().map(Convert::convert).collect()
}

fn default_variant_option(option: StringView<'_>) -> QVariant {
    WorldConfig::default_variant_option(option.as_slice()).convert()
}

fn encode_naws(browser: &QAbstractScrollArea) -> QByteArray {
    const ADVANCE_SAMPLE: &str = "0123456789";
    const ADVANCE_SAMPLE_LEN: i32 = 10;

    let metrics = browser.font_metrics();
    let margins = browser.contents_margins();
    let advance = metrics.horizontal_advance(&QString::from(ADVANCE_SAMPLE));
    let viewport = browser.maximum_viewport_size();
    let client_width = viewport.width() - margins.left() - margins.right();
    let client_height = viewport.height() - margins.top() - margins.bottom();
    let width = client_width * ADVANCE_SAMPLE_LEN / advance;
    let height = client_height / metrics.line_spacing() - 4;

    #[allow(clippy::cast_sign_loss, clippy::cast_possible_truncation)]
    QByteArray::from(naws(width as u16, height as u16).as_slice())
}

fn fixup_html(text: StringView<'_>) -> String {
    smushclient::fixup_html(text.as_slice())
}

fn get_alpha_option_list() -> QStringList {
    let mut list = QStringList::default();
    list.extend(
        smushclient::WorldConfig::STR_OPTIONS
            .iter()
            .map(|&s| QString::from(s)),
    );
    list
}

fn get_global_entity(name: StringView<'_>) -> VariableView {
    mxp::Entity::global(name.as_slice()).into()
}

fn get_option_list() -> QStringList {
    let mut list = QStringList::default();
    list.extend(
        smushclient::WorldConfig::INT_OPTIONS
            .iter()
            .map(|&s| QString::from(s)),
    );
    list
}
