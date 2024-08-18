#![allow(clippy::cast_possible_wrap)]
#![allow(clippy::cast_sign_loss)]
#![allow(clippy::missing_safety_doc)]
#![allow(clippy::unnecessary_box_returns)]

#[macro_use]
mod convert;

mod client;
use client::SmushClientRust;

mod handler;

mod impls;

mod output;
use output::{RustOutputFragment, RustTelnetFragment, RustTextFragment};

mod sync;

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

    unsafe extern "C++" {
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
    }

    enum Heading {
        H1,
        H2,
        H3,
        H4,
        H5,
        H6,
        Normal,
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
    }

    enum SendTo {
        World,
        Input,
        Internet,
    }

    struct MxpLink {
        action: QString,
        hint: QString,
        prompts: QStringList,
        sendto: SendTo,
    }

    struct MxpEntitySet {
        name: QString,
        value: QString,
        publish: bool,
        is_variable: bool,
    }

    struct MxpEntityUnset {
        name: QString,
        is_variable: bool,
    }

    extern "Rust" {
        type RustTextFragment;
        fn text(&self) -> QString;
        fn foreground(&self) -> QColor;
        fn background(&self) -> QColor;
        fn is_blink(&self) -> bool;
        fn is_bold(&self) -> bool;
        fn is_highlight(&self) -> bool;
        fn is_inverse(&self) -> bool;
        fn is_italic(&self) -> bool;
        fn is_strikeout(&self) -> bool;
        fn is_underline(&self) -> bool;
        fn has_link(&self) -> bool;
        fn link(&self) -> MxpLink;
        fn has_font(&self) -> bool;
        fn font(&self) -> QString;
        fn has_size(&self) -> bool;
        fn size(&self) -> u8;
        fn is_heading(&self) -> bool;
        fn heading(&self) -> Heading;
    }

    enum TelnetRequest {
        Afk,
        Do,
        IacGa,
        Naws,
        Subnegotiation,
        Will,
    }

    extern "Rust" {
        type RustTelnetFragment;
        fn kind(&self) -> TelnetRequest;
        fn code(&self) -> u8;
        fn data(&self) -> QByteArray;
    }

    #[derive(Debug)]
    enum OutputKind {
        Effect,
        Hr,
        LineBreak,
        MxpError,
        MxpEntitySet,
        MxpEntityUnset,
        PageBreak,
        Telnet,
        Text,
    }

    extern "Rust" {
        type RustOutputFragment;
        fn kind(&self) -> OutputKind;
        fn mxp_entity_set(&self) -> MxpEntitySet;
        fn mxp_entity_unset(&self) -> MxpEntityUnset;
        fn mxp_error(&self) -> QString;
        fn telnet(&self) -> &RustTelnetFragment;
        fn text(&self) -> &RustTextFragment;
    }

    unsafe extern "C++Qt" {
        include!(<QtCore/QIODevice>);
        type QIODevice;

        unsafe fn read(self: Pin<&mut QIODevice>, data: *mut c_char, max_size: i64) -> i64;
        unsafe fn write(self: Pin<&mut QIODevice>, data: *const c_char, max_size: i64) -> i64;
    }

    unsafe extern "RustQt" {
        #[qobject]
        type SmushClient = super::SmushClientRust;
    }

    unsafe impl !cxx_qt::Locking for SmushClient {}

    unsafe extern "RustQt" {
        unsafe fn read(self: Pin<&mut SmushClient>, device: Pin<&mut QIODevice>) -> i64;
        fn try_next(self: Pin<&mut SmushClient>) -> bool;
        fn next(self: &SmushClient) -> &RustOutputFragment;
    }
}
