use std::io::BufRead;
use std::os::raw::c_char;
use std::pin::Pin;
use std::sync::atomic::{AtomicBool, Ordering};

use cxx_qt_lib::{QByteArray, QColor, QList, QString, QStringList};
use mud_transformer::mxp::{self, SendTo, WorldColor};
use mud_transformer::{
    EffectFragment, OutputFragment, TelnetFragment, TextFragment, TextStyle, Transformer,
};

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
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
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
        Image,
        LineBreak,
        PageBreak,
        Telnet,
        Text,
    }

    extern "Rust" {
        type RustOutputFragment;
        fn kind(&self) -> OutputKind;
        fn effect(&self) -> EffectFragment;
        fn image(&self) -> QString;
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

fn encode_color(color: WorldColor) -> QColor {
    match color {
        WorldColor::Ansi(_) => unreachable!(),
        WorldColor::Hex(color) => {
            QColor::from_rgb(color.r() as i32, color.g() as i32, color.b() as i32)
        }
    }
}

impl From<&mxp::Link> for ffi::MxpLink {
    fn from(action: &mxp::Link) -> Self {
        let hint = match action.hint {
            Some(ref hint) => hint.as_str().into(),
            None => QString::default(),
        };
        let mut prompts = QList::default();
        for prompt in &action.prompts {
            prompts.append(QString::from(prompt));
        }
        Self {
            action: action.action.as_str().into(),
            hint,
            prompts: QStringList::from(&prompts),
            sendto: action.sendto.into(),
        }
    }
}

#[repr(transparent)]
struct RustTextFragment {
    inner: TextFragment,
}

impl From<TextFragment> for RustTextFragment {
    fn from(inner: TextFragment) -> Self {
        Self { inner }
    }
}

macro_rules! flag_method {
    ($n:ident, $v:expr) => {
        #[inline]
        fn $n(&self) -> bool {
            self.inner.flags.contains($v)
        }
    };
}

impl RustTextFragment {
    #[inline]
    fn text(&self) -> QString {
        QString::from(&*self.inner.text)
    }

    #[inline]
    fn foreground(&self) -> QColor {
        encode_color(self.inner.foreground)
    }

    #[inline]
    fn background(&self) -> QColor {
        encode_color(self.inner.background)
    }

    #[inline]
    fn has_link(&self) -> bool {
        self.inner.action.is_some()
    }

    #[inline]
    fn link(&self) -> ffi::MxpLink {
        self.inner.action.as_ref().unwrap().as_ref().into()
    }

    flag_method!(is_blink, TextStyle::Blink);
    flag_method!(is_bold, TextStyle::Bold);
    flag_method!(is_highlight, TextStyle::Highlight);
    flag_method!(is_inverse, TextStyle::Inverse);
    flag_method!(is_italic, TextStyle::Italic);
    flag_method!(is_strikeout, TextStyle::Strikeout);
    flag_method!(is_underline, TextStyle::Underline);
}

macro_rules! impl_enum_from {
    ($f:ty, $t:path, $($variant:ident),+ $(,)?) => {
        impl From<$t> for $f {
            fn from(value: $t) -> Self {
                match value {
                    $(<$t>::$variant => Self::$variant),+
                }
            }
        }
    }
}

impl_enum_from!(ffi::SendTo, SendTo, World, Input, Internet);

impl_enum_from!(
    ffi::EffectFragment,
    EffectFragment,
    Backspace,
    Beep,
    CarriageReturn,
    EraseCharacter,
    EraseLine
);

#[repr(transparent)]
struct RustTelnetFragment {
    inner: TelnetFragment,
}

impl From<TelnetFragment> for RustTelnetFragment {
    fn from(inner: TelnetFragment) -> Self {
        Self { inner }
    }
}

impl RustTelnetFragment {
    fn kind(&self) -> ffi::TelnetRequest {
        match self.inner {
            TelnetFragment::Afk { .. } => ffi::TelnetRequest::Afk,
            TelnetFragment::Do { .. } => ffi::TelnetRequest::Do,
            TelnetFragment::IacGa => ffi::TelnetRequest::IacGa,
            TelnetFragment::Naws => ffi::TelnetRequest::Naws,
            TelnetFragment::Subnegotiation { .. } => ffi::TelnetRequest::Subnegotiation,
            TelnetFragment::Will { .. } => ffi::TelnetRequest::Will,
        }
    }

    fn code(&self) -> u8 {
        match self.inner {
            TelnetFragment::Do { code } => code,
            TelnetFragment::Subnegotiation { code, .. } => code,
            TelnetFragment::Will { code } => code,
            _ => 0,
        }
    }

    fn data(&self) -> QByteArray {
        match &self.inner {
            TelnetFragment::Afk { challenge } => QByteArray::from(&**challenge),
            TelnetFragment::Subnegotiation { data, .. } => QByteArray::from(&**data),
            _ => QByteArray::default(),
        }
    }
}

#[repr(transparent)]
struct OutputFragmentRust {
    inner: OutputFragment,
}

impl From<OutputFragment> for OutputFragmentRust {
    fn from(inner: OutputFragment) -> Self {
        Self { inner }
    }
}

enum RustOutputFragment {
    Effect(ffi::EffectFragment),
    Hr,
    Image(String),
    LineBreak,
    PageBreak,
    Telnet(RustTelnetFragment),
    Text(RustTextFragment),
}

impl From<OutputFragment> for RustOutputFragment {
    fn from(value: OutputFragment) -> Self {
        match value {
            OutputFragment::Effect(effect) => Self::Effect(effect.into()),
            OutputFragment::Hr => Self::Hr,
            OutputFragment::Image(src) => Self::Image(src),
            OutputFragment::LineBreak => Self::LineBreak,
            OutputFragment::PageBreak => Self::PageBreak,
            OutputFragment::Telnet(telnet) => Self::Telnet(telnet.into()),
            OutputFragment::Text(text) => Self::Text(text.into()),
        }
    }
}

impl RustOutputFragment {
    fn kind(&self) -> ffi::OutputKind {
        match self {
            RustOutputFragment::Effect(_) => ffi::OutputKind::Effect,
            RustOutputFragment::Hr => ffi::OutputKind::Hr,
            RustOutputFragment::Image(_) => ffi::OutputKind::Image,
            RustOutputFragment::LineBreak => ffi::OutputKind::LineBreak,
            RustOutputFragment::PageBreak => ffi::OutputKind::PageBreak,
            RustOutputFragment::Telnet(_) => ffi::OutputKind::Telnet,
            RustOutputFragment::Text(_) => ffi::OutputKind::Text,
        }
    }

    fn effect(&self) -> ffi::EffectFragment {
        match self {
            RustOutputFragment::Effect(fragment) => *fragment,
            _ => panic!("expected Effect, found {:?}", self.kind()),
        }
    }

    fn image(&self) -> QString {
        match self {
            RustOutputFragment::Image(image) => QString::from(image),
            _ => panic!("expected Image, found {:?}", self.kind()),
        }
    }

    fn telnet(&self) -> &RustTelnetFragment {
        match self {
            RustOutputFragment::Telnet(request) => request,
            _ => panic!("expected Telnet, found {:?}", self.kind()),
        }
    }

    fn text(&self) -> &RustTextFragment {
        match self {
            RustOutputFragment::Text(text) => text,
            _ => panic!("expected Text, found {:?}", self.kind()),
        }
    }
}

#[derive(Default)]
#[repr(transparent)]
pub struct SimpleLock {
    locked: AtomicBool,
}

impl SimpleLock {
    fn lock(&self) -> SimpleLockGuard {
        if self.locked.swap(true, Ordering::Relaxed) {
            panic!("concurrent access");
        }
        SimpleLockGuard {
            locked: &self.locked,
        }
    }
}

pub struct SimpleLockGuard<'a> {
    locked: &'a AtomicBool,
}

impl<'a> Drop for SimpleLockGuard<'a> {
    fn drop(&mut self) {
        self.locked.store(false, Ordering::Relaxed);
    }
}

#[derive(Default)]
pub struct SmushClientRust {
    done: bool,
    transformer: Transformer,
    buf: Vec<u8>,
    output: Vec<RustOutputFragment>,
    cursor: usize,
    input_lock: SimpleLock,
    output_lock: SimpleLock,
}

impl SmushClientRust {
    fn read(&mut self, mut device: Pin<&mut ffi::QIODevice>) -> i64 {
        if self.done {
            return -1;
        }
        if self.cursor >= self.output.len() {
            self.cursor = 0;
            self.output.clear();
        }

        let output_lock = self.output_lock.lock();
        let buf_ptr = self.buf.as_mut_ptr() as *mut c_char;
        let buf_len = self.buf.len() as i64;
        let mut total_read = 0;
        loop {
            // SAFETY: Device will not read past buf_len.
            let n = unsafe { device.as_mut().read(buf_ptr, buf_len) };
            if n == 0 {
                break;
            }
            total_read += n;
            if n == -1 {
                self.done = true;
                self.output
                    .extend(self.transformer.flush_output().map(Into::into));
                return total_read;
            }
            let (received, buf) = self.buf.split_at_mut(n as usize);
            self.transformer.receive(received, buf).unwrap();
        }
        self.output
            .extend(self.transformer.drain_output().map(Into::into));
        drop(output_lock);

        let input_lock = self.input_lock.lock();
        if let Some(mut drain) = self.transformer.drain_input() {
            loop {
                let drain_buf = drain.fill_buf().unwrap();
                if drain_buf.is_empty() {
                    break;
                }
                let drain_ptr = drain_buf.as_ptr() as *const c_char;
                let drain_len = drain_buf.len() as i64;
                // SAFETY: Device will not write past drain_len.
                let n = unsafe { device.as_mut().write(drain_ptr, drain_len) };
                drain.consume(n as usize);
            }
        }
        drop(input_lock);

        return total_read;
    }

    fn try_next(&mut self) -> bool {
        if self.cursor < self.output.len() {
            self.cursor += 1;
            true
        } else {
            false
        }
    }

    fn next(&self) -> &RustOutputFragment {
        &self.output[self.cursor - 1]
    }
}

impl ffi::SmushClient {
    fn read(self: Pin<&mut Self>, device: Pin<&mut ffi::QIODevice>) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device)
    }

    fn try_next(self: Pin<&mut Self>) -> bool {
        self.cxx_qt_ffi_rust_mut().try_next()
    }

    fn next(&self) -> &RustOutputFragment {
        self.cxx_qt_ffi_rust().next()
    }
}
