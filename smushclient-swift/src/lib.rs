#![allow(clippy::unnecessary_cast)]
use mud_stream::nonblocking::MudStream;
use mud_transformer::mxp::{self, SendTo, WorldColor};
use mud_transformer::{EffectFragment, TelnetFragment};
use mud_transformer::{OutputFragment, TextFragment, TextStyle};
use std::sync::atomic::{AtomicBool, Ordering};
use std::vec;
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

#[swift_bridge::bridge]
mod ffi {
    enum MudColor {
        Ansi(u8),
        Hex(u32),
    }

    enum SendTo {
        World,
        Input,
        Internet,
    }

    extern "Rust" {
        type RustMxpLink;
        fn action(&self) -> &str;
        fn hint(&self) -> Option<&str>;
        fn prompts(&self) -> Vec<String>;
        fn sendto(&self) -> SendTo;
    }

    extern "Rust" {
        type RustTextFragment;
        fn text(&self) -> &str;
        fn foreground(&self) -> MudColor;
        fn background(&self) -> MudColor;
        #[swift_bridge(swift_name = "isBlink")]
        fn is_blink(&self) -> bool;
        #[swift_bridge(swift_name = "isBold")]
        fn is_bold(&self) -> bool;
        #[swift_bridge(swift_name = "isHighlight")]
        fn is_highlight(&self) -> bool;
        #[swift_bridge(swift_name = "isInverse")]
        fn is_inverse(&self) -> bool;
        #[swift_bridge(swift_name = "IsItalic")]
        fn is_italic(&self) -> bool;
        #[swift_bridge(swift_name = "isStrikeout")]
        fn is_strikeout(&self) -> bool;
        #[swift_bridge(swift_name = "isUnderline")]
        fn is_underline(&self) -> bool;
        fn link(&self) -> Option<&RustMxpLink>;
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
    }

    enum TelnetFragment {
        Afk { challenge: String },
        Do { code: u8 },
        IacGa,
        Naws,
        Subnegotiation { code: u8, data: Vec<u8> },
        Will { code: u8 },
    }

    enum OutputFragment {
        Effect(EffectFragment),
        Hr,
        Image(String),
        LineBreak,
        PageBreak,
        Telnet(TelnetFragment),
        Text(RustTextFragment),
    }

    extern "Rust" {
        type RustOutputStream;
        fn next(&mut self) -> Option<OutputFragment>;
    }

    extern "Rust" {
        type RustMudBridge;
        #[swift_bridge(init)]
        fn new(address: String, port: u16) -> RustMudBridge;
        fn connected(&self) -> bool;
        async fn connect(&mut self) -> Result<(), String>;
        async fn disconnect(&mut self) -> Result<(), String>;
        async fn receive(&mut self) -> Result<RustOutputStream, String>;
        async fn send(&mut self, input: String) -> Result<(), String>;
    }
}

impl From<WorldColor> for ffi::MudColor {
    #[inline]
    fn from(value: WorldColor) -> Self {
        match value {
            mxp::WorldColor::Ansi(code) => Self::Ansi(code),
            mxp::WorldColor::Hex(color) => Self::Hex(color.code()),
        }
    }
}

#[repr(transparent)]
struct RustMxpLink {
    inner: mxp::Link,
}

impl RustMxpLink {
    fn cast(link: &mxp::Link) -> &Self {
        // SAFETY: #[repr(transparent)]
        unsafe { &*(link as *const mxp::Link as *const Self) }
    }

    fn action(&self) -> &str {
        &self.inner.action
    }

    fn hint(&self) -> Option<&str> {
        self.inner.hint.as_deref()
    }

    fn prompts(&self) -> Vec<String> {
        self.inner.prompts.clone()
    }

    fn sendto(&self) -> ffi::SendTo {
        self.inner.sendto.into()
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
        #[inline(always)]
        fn $n(&self) -> bool {
            self.inner.flags.contains($v)
        }
    };
}

impl RustTextFragment {
    #[inline(always)]
    fn text(&self) -> &str {
        &self.inner.text
    }

    #[inline(always)]
    fn foreground(&self) -> ffi::MudColor {
        self.inner.foreground.into()
    }

    #[inline(always)]
    fn background(&self) -> ffi::MudColor {
        self.inner.background.into()
    }

    #[inline]
    fn link(&self) -> Option<&RustMxpLink> {
        self.inner
            .action
            .as_ref()
            .map(|action| RustMxpLink::cast(action))
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

impl From<TelnetFragment> for ffi::TelnetFragment {
    fn from(value: TelnetFragment) -> Self {
        match value {
            TelnetFragment::Afk { challenge } => Self::Afk {
                challenge: String::from(&challenge),
            },
            TelnetFragment::Do { code } => Self::Do { code },
            TelnetFragment::IacGa => Self::IacGa,
            TelnetFragment::Naws => Self::Naws,
            TelnetFragment::Subnegotiation { code, data } => Self::Subnegotiation {
                code,
                data: data.to_vec(),
            },
            TelnetFragment::Will { code } => Self::Will { code },
        }
    }
}

impl From<OutputFragment> for ffi::OutputFragment {
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

#[repr(transparent)]
struct RustOutputStream {
    inner: vec::IntoIter<ffi::OutputFragment>,
}

impl RustOutputStream {
    #[inline(always)]
    fn next(&mut self) -> Option<ffi::OutputFragment> {
        self.inner.next()
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
pub struct RustMudBridge {
    address: String,
    port: u16,
    stream: Option<MudStream<TcpStream>>,
    input_lock: SimpleLock,
    output_lock: SimpleLock,
}

impl RustMudBridge {
    fn new(address: String, port: u16) -> Self {
        Self {
            address,
            port,
            ..Default::default()
        }
    }

    fn connected(&self) -> bool {
        self.stream.is_some()
    }

    async fn connect(&mut self) -> Result<(), String> {
        let stream = TcpStream::connect((self.address.clone(), self.port))
            .await
            .map_err(|e| e.to_string())?;
        let locks = (self.output_lock.lock(), self.input_lock.lock());
        self.stream = Some(MudStream::new(stream, Default::default()));
        drop(locks);
        Ok(())
    }

    async fn disconnect(&mut self) -> Result<(), String> {
        let locks = (self.output_lock.lock(), self.input_lock.lock());
        let result = match self.stream {
            Some(ref mut stream) => stream.shutdown().await.map_err(|e| e.to_string()),
            None => Ok(()),
        };
        drop(locks);
        result
    }

    async fn receive(&mut self) -> Result<RustOutputStream, String> {
        let lock = self.output_lock.lock();
        let result = match self.stream {
            Some(ref mut stream) => stream.read().await.map_err(|e| e.to_string())?,
            None => None,
        };
        let output = match result {
            Some(output) => output.map(Into::into).collect(),
            None => Vec::new(),
        };
        drop(lock);
        Ok(RustOutputStream {
            inner: output.into_iter(),
        })
    }

    async fn send(&mut self, input: String) -> Result<(), String> {
        let input = input.as_bytes();
        let lock = self.input_lock.lock();
        let result = match self.stream {
            Some(ref mut stream) => stream.write_all(input).await.map_err(|e| e.to_string()),
            None => Ok(()),
        };
        drop(lock);
        result
    }
}
