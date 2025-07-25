#![allow(clippy::elidable_lifetime_names)]
use std::ops::Range;
use std::pin::Pin;

use crate::colors::QColorPair;
use crate::sender::{OutputSpan, TextSpan};
use cxx::type_id;
use cxx_qt_lib::{QFlag, QFlags, QString};
use flagset::FlagSet;
use mud_transformer::{TelnetSource, TelnetVerb, TextStyle};

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qbytearray.h");
        type QByteArray = cxx_qt_lib::QByteArray;
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    enum TextStyle {
        Blink = 1,
        Bold = 2,
        Highlight = 4,
        Italic = 8,
        NonProportional = 16,
        Small = 32,
        Strikeout = 64,
        Underline = 128,
        Inverse = 256,
    }

    enum TelnetSource {
        Client,
        Server,
    }

    enum TelnetVerb {
        Do,
        Dont,
        Will,
        Wont,
    }

    enum SendTo {
        Internet,
        World,
        Input,
    }

    struct Link<'a> {
        action: QString,
        hint: QString,
        prompts: QString,
        sendto: SendTo,
        expires: &'a str,
    }

    struct SendRequest {
        plugin: usize,
        send_to: SendTarget,
        text: QString,
        destination: QString,
    }

    struct NamedWildcard<'a> {
        name: &'a str,
        value: &'a str,
    }

    struct SendScriptRequest<'a> {
        plugin: usize,
        script: &'a str,
        label: &'a str,
        line: &'a str,
        wildcards: Vec<&'a str>,
        #[rust_name = "named_wildcards"]
        namedWildcards: Vec<NamedWildcard<'a>>,
        output: &'a [OutputSpan],
    }

    extern "Rust" {
        type TextSpan;
        fn foreground(&self) -> i32;
        fn background(&self) -> i32;
        fn text(&self) -> &str;
        fn style(&self) -> u8;
    }

    extern "Rust" {
        type OutputSpan;
        fn text_span(&self) -> *const TextSpan;
    }

    unsafe extern "C++Qt" {
        include!("document.h");
        type TextStyles = super::TextStyles;
        type SendTarget = crate::ffi::SendTarget;

        #[qobject]
        type Document;

        #[rust_name = "append_html"]
        fn appendHtml(self: &Document, text: &QString);

        #[rust_name = "append_line"]
        fn appendLine(self: Pin<&mut Document>);

        #[rust_name = "append_plaintext"]
        fn appendText(self: &Document, text: &QString, palette: i32);

        #[rust_name = "append_text_internal"]
        fn appendText(
            self: &Document,
            text: &QString,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
        );

        #[rust_name = "append_link_internal"]
        fn appendText(
            self: Pin<&mut Document>,
            text: &QString,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
            link: &Link,
        );

        #[rust_name = "apply_styles_internal"]
        fn applyStyles(
            self: &Document,
            start: i32,
            end: i32,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
        );

        fn beep(self: &Document);

        fn begin(self: &Document);

        #[rust_name = "create_mxp_stat"]
        fn createMxpStat(self: &Document, entity: &QString, caption: &QString, max: &QString);

        fn end(self: Pin<&mut Document>, had_output: bool);

        #[rust_name = "erase_current_line"]
        fn eraseCurrentLine(self: &Document);

        #[rust_name = "erase_last_character"]
        fn eraseLastCharacter(self: &Document);

        #[rust_name = "erase_last_line"]
        fn eraseLastLine(self: &Document);

        #[rust_name = "expire_links"]
        fn expireLinks(self: Pin<&mut Document>, expires: &str);

        #[rust_name = "handle_mxp_change"]
        fn handleMxpChange(self: &Document, enabled: bool);

        #[rust_name = "handle_mxp_entity"]
        fn handleMxpEntity(self: &Document, data: &str);

        #[rust_name = "handle_mxp_variable"]
        fn handleMxpVariable(self: &Document, name: &str, value: &str);

        #[rust_name = "handle_server_status"]
        fn handleServerStatus(self: Pin<&mut Document>, variable: &QByteArray, value: &QByteArray);

        #[rust_name = "handle_telnet_go_ahead"]
        fn handleTelnetGoAhead(self: &Document);

        #[rust_name = "handle_telnet_naws"]
        fn handleTelnetNaws(self: &Document);

        #[rust_name = "handle_telnet_negotiation_internal"]
        fn handleTelnetNegotiation(
            self: Pin<&mut Document>,
            source: TelnetSource,
            verb: TelnetVerb,
            code: u8,
        );

        #[rust_name = "handle_telnet_subnegotiation"]
        fn handleTelnetSubnegotiation(self: &Document, code: u8, data: &QByteArray);

        #[rust_name = "permit_line"]
        fn permitLine(self: &Document, line: &str) -> bool;

        fn send(self: &Document, request: &SendRequest);

        #[rust_name = "send_script"]
        fn send(self: &Document, request: &SendScriptRequest);

        #[rust_name = "set_suppress_echo"]
        fn setSuppressEcho(self: &Document, suppress: bool);

        #[rust_name = "update_mxp_stat"]
        fn updateMxpStat(self: &Document, entity: &QString, value: &QString);
    }
}

impl ffi::Document {
    pub fn append_text(&self, text: &QString, style: FlagSet<TextStyle>, color: &QColorPair) {
        self.append_text_internal(
            text,
            ffi::TextStyle::to_qflags(style),
            &color.foreground,
            &color.background,
        );
    }

    pub fn append_link(
        self: Pin<&mut Self>,
        text: &QString,
        style: FlagSet<TextStyle>,
        color: &QColorPair,
        link: &ffi::Link,
    ) {
        self.append_link_internal(
            text,
            ffi::TextStyle::to_qflags(style),
            &color.foreground,
            &color.background,
            link,
        );
    }

    pub fn apply_styles(&self, range: Range<usize>, style: FlagSet<TextStyle>, color: &QColorPair) {
        self.apply_styles_internal(
            i32::try_from(range.start).unwrap_or(i32::MAX),
            i32::try_from(range.end - range.start).unwrap_or(i32::MAX),
            ffi::TextStyle::to_qflags(style),
            &color.foreground,
            &color.background,
        );
    }

    pub fn handle_telnet_negotiation(
        self: Pin<&mut Self>,
        source: TelnetSource,
        verb: TelnetVerb,
        code: u8,
    ) {
        self.handle_telnet_negotiation_internal(source.into(), verb.into(), code);
    }
}

pub type TextStyles = QFlags<ffi::TextStyle>;

impl ffi::TextStyle {
    pub fn to_qflags(styles: FlagSet<TextStyle>) -> TextStyles {
        TextStyles::from_int(styles.bits().into())
    }
}

// SAFETY: Static checks on C++ side.
unsafe impl QFlag for ffi::TextStyle {
    type TypeId = type_id!("TextStyles");

    type Repr = u16;

    fn to_repr(self) -> Self::Repr {
        self.repr
    }
}

macro_rules! assert_textstyle {
    ($i:ident) => {
        const _: () = assert!(1 << (TextStyle::$i as u16) == ffi::TextStyle::$i.repr as usize);
    };
}

assert_textstyle!(Blink);
assert_textstyle!(Bold);
assert_textstyle!(Highlight);
assert_textstyle!(NonProportional);
assert_textstyle!(Small);
assert_textstyle!(Strikeout);
assert_textstyle!(Underline);
assert_textstyle!(Inverse);
