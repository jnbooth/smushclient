#![allow(clippy::elidable_lifetime_names)]
use std::pin::Pin;

use mud_transformer::{TelnetSource, TelnetVerb, TextStyle};

use crate::sender::{OutputSpan, TextSpan};

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

    extern "C++" {
        include!("smushclient_qt/src/ffi/send_request.cxx.h");
        type SendRequest = super::super::send_request::ffi::SendRequest;
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
        #[cxx_name = "namedWildcards"]
        named_wildcards: Vec<NamedWildcard<'a>>,
        output: &'a [OutputSpan],
    }

    extern "C++" {
        include!("smushclient-qt-lib/qtextcharformat.h");
        type QTextCharFormat = smushclient_qt_lib::QTextCharFormat;
    }

    #[namespace = "rust::smushclientqtlib1"]
    extern "C++" {

        include!("smushclient-qt-lib/qtextcursor.h");
        type QTextCursorMoveOperation = smushclient_qt_lib::QTextCursorMoveOperation;
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
        include!("smushclient_qt/document.h");
        type SendTarget = crate::ffi::SendTarget;

        #[qobject]
        type Document;

        #[rust_name = "append_html"]
        fn appendHtml(self: &Document, text: &QString);

        #[rust_name = "append_expiring_link"]
        fn appendExpiringLink(
            self: Pin<&mut Document>,
            text: &QString,
            format: &QTextCharFormat,
            expires: &str,
        );

        #[rust_name = "append_line"]
        fn appendLine(self: Pin<&mut Document>);

        #[rust_name = "append_text"]
        fn appendText(self: &Document, text: &QString, format: &QTextCharFormat);

        #[rust_name = "apply_styles"]
        fn applyStyles(self: &Document, start: i32, end: i32, format: &QTextCharFormat);

        fn beep(self: &Document);

        fn begin(self: &Document);

        fn clear(self: &Document);

        #[rust_name = "create_mxp_stat"]
        fn createMxpStat(self: &Document, entity: &QString, caption: &QString, max: &QString);

        fn echo(self: &Document, text: &QString);

        fn end(self: Pin<&mut Document>, had_output: bool);

        #[rust_name = "erase_current_line"]
        fn eraseCurrentLine(self: &Document);

        #[rust_name = "erase_characters"]
        fn eraseCharacters(self: &Document, direction: QTextCursorMoveOperation, n: i32);

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

        #[rust_name = "move_cursor"]
        fn moveCursor(self: &Document, op: QTextCursorMoveOperation, count: i32);

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
    pub fn handle_telnet_negotiation(
        self: Pin<&mut Self>,
        source: TelnetSource,
        verb: TelnetVerb,
        code: u8,
    ) {
        self.handle_telnet_negotiation_internal(source.into(), verb.into(), code);
    }
}

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
