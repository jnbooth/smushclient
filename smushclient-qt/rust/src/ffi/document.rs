use std::mem;

use crate::sender::{OutputSpan, TextSpan};
use cxx::{type_id, ExternType};

#[repr(transparent)]
pub struct TextStyles(pub u16);
const _: [(); mem::size_of::<TextStyles>()] = [(); mem::size_of::<ffi::TextStyle>()];

unsafe impl ExternType for TextStyles {
    type Id = type_id!("TextStyles");
    type Kind = cxx::kind::Trivial;
}

#[cxx_qt::bridge]
pub mod ffi {
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

    extern "C++Qt" {
        include!("document.h");
        type TextStyles = super::TextStyles;
        type SendTarget = crate::ffi::SendTarget;

        #[qobject]
        type Document;

        #[rust_name = "append_html"]
        unsafe fn appendHtml(self: &Document, text: &QString);

        #[rust_name = "append_line"]
        unsafe fn appendLine(self: Pin<&mut Document>);

        #[rust_name = "append_plaintext"]
        unsafe fn appendText(self: &Document, text: &QString, palette: i32);

        #[rust_name = "append_text"]
        unsafe fn appendText(
            self: &Document,
            text: &QString,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
        );

        #[rust_name = "append_link"]
        unsafe fn appendText(
            self: Pin<&mut Document>,
            text: &QString,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
            link: &Link,
        );

        #[rust_name = "apply_styles"]
        unsafe fn applyStyles(
            self: &Document,
            start: i32,
            end: i32,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
        );

        unsafe fn beep(self: &Document);

        unsafe fn begin(self: &Document);

        #[rust_name = "create_mxp_stat"]
        unsafe fn createMxpStat(
            self: &Document,
            entity: &QString,
            caption: &QString,
            max: &QString,
        );

        unsafe fn end(self: Pin<&mut Document>, had_output: bool);

        #[rust_name = "erase_current_line"]
        unsafe fn eraseCurrentLine(self: &Document);

        #[rust_name = "erase_last_character"]
        unsafe fn eraseLastCharacter(self: &Document);

        #[rust_name = "erase_last_line"]
        unsafe fn eraseLastLine(self: &Document);

        #[rust_name = "expire_links"]
        unsafe fn expireLinks(self: Pin<&mut Document>, expires: &str);

        #[rust_name = "handle_mxp_change"]
        unsafe fn handleMxpChange(self: &Document, enabled: bool);

        #[rust_name = "handle_mxp_entity"]
        unsafe fn handleMxpEntity(self: &Document, data: &str);

        #[rust_name = "handle_mxp_variable"]
        unsafe fn handleMxpVariable(self: &Document, name: &str, value: &str);

        #[rust_name = "handle_server_status"]
        unsafe fn handleServerStatus(
            self: Pin<&mut Document>,
            variable: &QByteArray,
            value: &QByteArray,
        );

        #[rust_name = "handle_telnet_go_ahead"]
        unsafe fn handleTelnetGoAhead(self: &Document);

        #[rust_name = "handle_telnet_naws"]
        unsafe fn handleTelnetNaws(self: &Document);

        #[rust_name = "handle_telnet_negotiation"]
        unsafe fn handleTelnetNegotiation(
            self: Pin<&mut Document>,
            source: TelnetSource,
            verb: TelnetVerb,
            code: u8,
        );

        #[rust_name = "handle_telnet_subnegotiation"]
        unsafe fn handleTelnetSubnegotiation(self: &Document, code: u8, data: &QByteArray);

        #[rust_name = "permit_line"]
        unsafe fn permitLine(self: &Document, line: &str) -> bool;

        #[rust_name = "play_sound"]
        unsafe fn playSound(self: &Document, filePath: &QString);

        unsafe fn send(self: &Document, request: &SendRequest);

        #[rust_name = "send_script"]
        unsafe fn send(self: &Document, request: &SendScriptRequest);

        #[rust_name = "set_suppress_echo"]
        unsafe fn setSuppressEcho(self: &Document, suppress: bool);

        #[rust_name = "update_mxp_stat"]
        unsafe fn updateMxpStat(self: &Document, entity: &QString, value: &QString);
    }
}
