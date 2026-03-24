use cxx_qt_lib::QString;
use flagset::FlagSet;
use mud_transformer::output::{Link, SendTo, TextStyle};
use smushclient_qt_lib::{QFontWeight, QTextCharFormat, QTextFormatProperty};

#[cxx::bridge]
mod ffi {
    #[repr(u8)]
    enum SendTo {
        Internet,
        World,
        Prompt,
    }

    #[namespace = "ffi::spans"]
    #[repr(i32)]
    enum SpanProperty {
        Styles = 0x100000,
        SendTo,
        Prompts,
        LineType,
    }

    enum TextStyle {
        NonProportional = 1,
        Bold = 2,
        Faint = 4,
        Italic = 8,
        Underline = 16,
        Blink = 32,
        Small = 64,
        Inverse = 128,
        Conceal = 256,
        Strikeout = 512,
        Highlight = 1024,
    }

    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;

        include!("smushclient-qt-lib/qtextcharformat.h");
        type QTextCharFormat = smushclient_qt_lib::QTextCharFormat;
    }
}

struct SpanProperty;

impl SpanProperty {
    pub const STYLES: QTextFormatProperty = QTextFormatProperty {
        repr: ffi::SpanProperty::Styles.repr,
    };
    pub const SEND_TO: QTextFormatProperty = QTextFormatProperty {
        repr: ffi::SpanProperty::SendTo.repr,
    };
    pub const PROMPTS: QTextFormatProperty = QTextFormatProperty {
        repr: ffi::SpanProperty::Prompts.repr,
    };
}

impl From<SendTo> for ffi::SendTo {
    fn from(value: SendTo) -> Self {
        match value {
            SendTo::World => Self::World,
            SendTo::Prompt => Self::Prompt,
            SendTo::Internet => Self::Internet,
        }
    }
}

// Styles

pub fn apply_styles(format: &mut QTextCharFormat, flags: FlagSet<TextStyle>) {
    format.set_property(SpanProperty::STYLES, &u32::from(flags.bits()));
    if flags.contains(TextStyle::Bold) {
        format.set_font_weight(QFontWeight::Bold);
    }
    if flags.contains(TextStyle::Italic) {
        format.set_font_italic(true);
    }
    if flags.contains(TextStyle::Strikeout) {
        format.set_font_strike_out(true);
    }
    if flags.contains(TextStyle::Underline) {
        format.set_font_underline(true);
    }
}

// Links

pub fn apply_link(format: &mut QTextCharFormat, link: &Link) {
    format.set_anchor(true);
    format.set_anchor_href(&QString::from(&link.href));
    format.set_property(SpanProperty::SEND_TO, &ffi::SendTo::from(link.send_to).repr);
    if link.hint.is_empty() {
        return;
    }
    if !link.menu {
        format.set_tool_tip(&QString::from(&link.hint));
        return;
    }
    let (tooltip, labels) = match link.hint.split_once('|') {
        Some((tooltip, labels)) => (QString::from(tooltip), QString::from(labels)),
        _ => (QString::from(&link.hint), QString::default()),
    };
    format.set_tool_tip(&tooltip);
    format.set_property(SpanProperty::PROMPTS, &labels);
}

// Assertions

macro_rules! assert_textstyle {
    ($i:ident) => {
        const _: () = assert!(1 << (TextStyle::$i as u16) == ffi::TextStyle::$i.repr as usize);
    };
}

assert_textstyle!(NonProportional);
assert_textstyle!(Bold);
assert_textstyle!(Faint);
assert_textstyle!(Italic);
assert_textstyle!(Underline);
assert_textstyle!(Blink);
assert_textstyle!(Small);
assert_textstyle!(Inverse);
assert_textstyle!(Conceal);
assert_textstyle!(Strikeout);
assert_textstyle!(Highlight);
