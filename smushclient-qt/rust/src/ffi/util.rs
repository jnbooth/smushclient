use cxx_qt_lib::{QByteArray, QColor, QString, QStringList, QVariant, QVector};
use mud_transformer::mxp::{self, RgbColor};
use mud_transformer::naws;
use smushclient::{WorldConfig, speedwalk};
use smushclient_qt_lib::QAbstractScrollArea;

use crate::convert::Convert;
use crate::ffi::{StringView, VariableView};
use crate::get_info::font_info;

#[allow(clippy::extra_unused_lifetimes)]
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
        fn decode_utf8(text: StringView) -> Vec<u32>;
        fn default_variant_option(option: StringView) -> QVariant;
        fn encode_naws(browser: &QAbstractScrollArea) -> QByteArray;
        fn encode_utf8(buf: &mut String, code: i64) -> bool;
        fn fixup_html(text: StringView) -> String;
        fn font_info(font: &QFont, info_type: i64) -> QVariant;
        fn get_alpha_option_list() -> QStringList;
        fn get_global_entity(name: StringView) -> VariableView;
        fn get_option_list() -> QStringList;
        fn is_utf8_valid(text: StringView<'_>) -> bool;
        fn reverse_speedwalk(text: StringView<'_>) -> String;
        unsafe fn utf8_substring<'a>(
            text: StringView<'a>,
            start: i64,
            end: i64,
            error_pos: &mut usize,
        ) -> &'a str;
    }
}

fn ansi16() -> QVector<QColor> {
    RgbColor::XTERM_16.iter().map(Convert::convert).collect()
}

fn decode_utf8(text: StringView<'_>) -> Vec<u32> {
    let Ok(text) = text.to_str() else {
        return Vec::new();
    };
    text.chars().map(u32::from).collect()
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

fn encode_utf8(buf: &mut String, code: i64) -> bool {
    let Some(c) = code.try_into().ok().and_then(char::from_u32) else {
        return false;
    };
    buf.push(c);
    true
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

fn is_utf8_valid(text: StringView<'_>) -> bool {
    text.to_str().is_ok()
}

fn reverse_speedwalk(text: StringView<'_>) -> String {
    let text = match text.to_str() {
        Ok(text) => text,
        Err(e) => return format!("*{e}"),
    };
    match speedwalk::reverse_to_string(text) {
        Ok(result) => result,
        Err(e) => format!("*{e}"),
    }
}

fn utf8_substring<'a>(
    text: StringView<'a>,
    mut start: i64,
    mut end: i64,
    error_pos: &mut usize,
) -> &'a str {
    let text = match text.to_str() {
        Ok(text) => text,
        Err(e) => {
            *error_pos = e.valid_up_to();
            return "";
        }
    };
    let text_len = text.char_indices().count();
    #[allow(clippy::cast_possible_wrap)]
    let text_len_signed = text_len as i64;
    if start < 0 {
        start += text_len_signed;
    }
    if end < 0 {
        end += text_len_signed;
    }
    let start = usize::try_from(start - 1).unwrap_or_default();
    let end = usize::try_from(end).unwrap_or_default();
    if start >= end {
        return "";
    }
    let Some((start_index, _)) = text.char_indices().nth(start) else {
        return "";
    };
    let end_index = match text.char_indices().nth(end) {
        Some((end_index, _)) => end_index,
        None => text.len(),
    };
    &text[start_index..end_index]
}
