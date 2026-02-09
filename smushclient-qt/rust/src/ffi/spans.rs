use std::fmt::Write;

use cxx_qt_lib::{CaseSensitivity, QString, QStringList, SplitBehaviorFlags};
use flagset::FlagSet;
use mud_transformer::TextStyle;
use mud_transformer::mxp;
use smushclient_qt_lib::QChar;
use smushclient_qt_lib::QStringExt;
use smushclient_qt_lib::{QFontWeight, QTextCharFormat, QTextFormatProperty};

#[cxx::bridge]
mod ffi {
    #[repr(u8)]
    enum SendTo {
        World = 0x20,
        Input = 0x21,
        Internet = 0x23,
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

    #[namespace = "ffi"]
    extern "Rust" {
        fn encode_link(send_to: SendTo, action: &mut QString);
        fn get_prompts(format: &QTextCharFormat) -> QStringList;
        fn get_styles(format: &QTextCharFormat) -> u32;
        fn get_send_to(link: &mut QString) -> SendTo;
    }
}

pub use ffi::SendTo;

struct SpanProperty;

impl SpanProperty {
    pub const STYLES: QTextFormatProperty = QTextFormatProperty::user(0);
    pub const PROMPTS: QTextFormatProperty = QTextFormatProperty::user(1);
}

// Styles

fn get_styles(format: &QTextCharFormat) -> u32 {
    format.property(SpanProperty::STYLES).value_or_default()
}

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

// Prompts

const SEP: &str = "\x1E";
const SEP_CHAR: QChar = QChar::new(0x1E);

fn get_prompts(format: &QTextCharFormat) -> QStringList {
    let property = format.property(SpanProperty::PROMPTS).value_or_default();
    decode_prompts(&property)
}

fn decode_prompts(encoded: &QString) -> QStringList {
    if encoded.is_empty() {
        return QStringList::default();
    }
    encoded.split_char(
        SEP_CHAR,
        SplitBehaviorFlags::KeepEmptyParts,
        CaseSensitivity::CaseSensitive,
    )
}

fn encode_prompts(prompts: &[mxp::LinkPrompt]) -> QString {
    let len = prompts
        .iter()
        .map(|prompt| prompt.action.len() + prompt.label().len() + SEP.len() + SEP.len())
        .sum();
    let mut s = String::with_capacity(len);
    for prompt in prompts {
        write!(s, "{SEP}{}{SEP}{}", &prompt.action, &prompt.label()).unwrap();
    }
    QString::from(&s[1..])
}

// Links

const WORLD: QChar = QChar::new(ffi::SendTo::World.repr as u16);
const INPUT: QChar = QChar::new(ffi::SendTo::Input.repr as u16);
const INTERNET: QChar = QChar::new(ffi::SendTo::Internet.repr as u16);

fn get_send_to(link: &mut QString) -> ffi::SendTo {
    println!("Link: {link}");
    let send_to = match link.back() {
        Some(WORLD) => ffi::SendTo::World,
        Some(INPUT) => ffi::SendTo::Input,
        Some(INTERNET) => ffi::SendTo::Internet,
        _ => {
            println!("Unrecognized link: {link}");
            return ffi::SendTo::Internet;
        }
    };
    link.remove_last();
    send_to
}

fn encode_link(send_to: ffi::SendTo, action: &mut QString) {
    action.append_char(match send_to {
        ffi::SendTo::World => WORLD,
        ffi::SendTo::Input => INPUT,
        ffi::SendTo::Internet => INTERNET,
        _ => return,
    });
}

fn encode_mxp_link(link: &mxp::Link) -> QString {
    let mut action = QString::from(&link.action);
    action.append_char(match link.send_to {
        mxp::SendTo::World => WORLD,
        mxp::SendTo::Input => INPUT,
        mxp::SendTo::Internet => INTERNET,
    });
    action
}

pub fn apply_link(format: &mut QTextCharFormat, link: &mxp::Link) {
    format.set_anchor(true);
    format.set_anchor_href(&encode_mxp_link(link));
    if let Some(hint) = &link.hint {
        format.set_tool_tip(&QString::from(hint));
    }
    if !link.prompts.is_empty() {
        format.set_property(SpanProperty::PROMPTS, &encode_prompts(&link.prompts));
    }
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
