#![allow(clippy::unnecessary_cast)]
use mud_stream::nonblocking::MudStream;
use mud_transformer::mxp::{self, Link, SendTo, WorldColor};
use mud_transformer::{EffectFragment, TelnetFragment};
use mud_transformer::{OutputFragment, TextFragment, TextStyle};
use std::future::Future;
use std::{io, vec};
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

    #[swift_bridge(swift_repr = "struct")]
    struct MxpLink {
        action: String,
        hint: String,
        prompts: Vec<String>,
        sendto: SendTo,
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
        fn link(&self) -> Option<MxpLink>;
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

pub struct RustOutputStream {
    inner: vec::IntoIter<ffi::OutputFragment>,
}

impl RustOutputStream {
    fn next(&mut self) -> Option<ffi::OutputFragment> {
        self.inner.next()
    }
}

pub struct RustMudBridge {
    address: String,
    port: u16,
    stream: Option<MudStream<TcpStream>>,
}

impl RustMudBridge {
    fn new(address: String, port: u16) -> Self {
        Self {
            address,
            port,
            stream: None,
        }
    }

    fn connected(&self) -> bool {
        self.stream.is_some()
    }

    async fn connect(&mut self) -> Result<(), String> {
        let stream = TcpStream::connect((self.address.clone(), self.port))
            .await
            .map_err(|e| e.to_string())?;
        self.stream = Some(MudStream::new(stream, Default::default()));
        Ok(())
    }

    #[inline]
    async fn with_stream<'a, T, Fut, F>(&'a mut self, mut f: F) -> Result<T, String>
    where
        T: Default,
        Fut: Future<Output = io::Result<T>> + Send,
        F: FnMut(&'a mut MudStream<TcpStream>) -> Fut,
    {
        match self.stream {
            Some(ref mut stream) => f(stream).await.map_err(|e| e.to_string()),
            None => Ok(T::default()),
        }
    }

    async fn receive(&mut self) -> Result<RustOutputStream, String> {
        let output = match self.with_stream(|stream| stream.read()).await? {
            Some(output) => output.map(ffi::OutputFragment::from).collect(),
            None => Vec::new(),
        };
        Ok(RustOutputStream {
            inner: output.into_iter(),
        })
    }

    async fn disconnect(&mut self) -> Result<(), String> {
        self.with_stream(|stream| stream.shutdown()).await
    }

    async fn send(&mut self, input: String) -> Result<(), String> {
        self.with_stream(|stream| stream.write_all(input.as_bytes()))
            .await
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
    fn text(&self) -> &str {
        &self.inner.text
    }

    #[inline]
    fn foreground(&self) -> ffi::MudColor {
        self.inner.foreground.into()
    }

    #[inline]
    fn background(&self) -> ffi::MudColor {
        self.inner.background.into()
    }

    #[inline]
    fn link(&self) -> Option<ffi::MxpLink> {
        match &self.inner.action {
            Some(action) => Some((**action).clone().into()),
            None => None,
        }
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

impl From<Link> for ffi::MxpLink {
    fn from(value: Link) -> Self {
        Self {
            action: value.action,
            hint: value.hint.unwrap_or_default(),
            prompts: value.prompts,
            sendto: value.sendto.into(),
        }
    }
}

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
            TelnetFragment::Afk { challenge } => ffi::TelnetFragment::Afk {
                challenge: String::from(&challenge),
            },
            TelnetFragment::Do { code } => ffi::TelnetFragment::Do { code },
            TelnetFragment::IacGa => ffi::TelnetFragment::IacGa,
            TelnetFragment::Naws => ffi::TelnetFragment::Naws,
            TelnetFragment::Subnegotiation { code, data } => ffi::TelnetFragment::Subnegotiation {
                code,
                data: data.to_vec(),
            },
            TelnetFragment::Will { code } => ffi::TelnetFragment::Will { code },
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
