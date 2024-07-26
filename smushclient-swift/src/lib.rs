#![allow(clippy::unnecessary_cast)]
use mud_stream::nonblocking::MudStream;
use mud_transformer::mxp::{self, Link, SendTo, WorldColor};
use mud_transformer::{EffectFragment, TelnetFragment};
use mud_transformer::{OutputFragment, TextFragment, TextStyle};
use std::io;
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;

use tokio::sync::mpsc;
use tokio::task::JoinHandle;

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
        fn text(&self) -> &[u8];
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

    enum PluginEvent {
        InputModified(String),
    }

    extern "Rust" {
        type RustMudBridge;
        #[swift_bridge(init)]
        fn new(address: String, port: u16) -> RustMudBridge;
        #[swift_bridge(swift_name = "isConnected")]
        fn is_connected(&self) -> bool;
        async fn connect(&mut self) -> Result<(), String>;
        fn disconnect(&self) -> bool;
        #[swift_bridge(swift_name = "getOutput")]
        async fn get_output(&mut self) -> Result<OutputFragment, String>;
        #[swift_bridge(swift_name = "sendInput")]
        fn send_input(&self, input: String) -> Result<(), String>;
        #[swift_bridge(swift_name = "waitUntilDone")]
        async fn wait_until_done(&mut self) -> Result<(), String>;
    }
}

pub struct RustMudBridge {
    address: String,
    port: u16,
    handle: Option<JoinHandle<io::Result<()>>>,
    input: mpsc::UnboundedSender<String>,
    output: mpsc::UnboundedReceiver<OutputFragment>,
    output_dispatch: mpsc::UnboundedSender<OutputFragment>,
}

impl RustMudBridge {
    fn new(address: String, port: u16) -> Self {
        let (input, _) = mpsc::unbounded_channel();
        let (output_dispatch, output) = mpsc::unbounded_channel();
        Self {
            address,
            port,
            handle: None,
            input,
            output,
            output_dispatch,
        }
    }

    fn is_connected(&self) -> bool {
        match &self.handle {
            Some(handle) => !handle.is_finished(),
            None => false,
        }
    }

    async fn connect(&mut self) -> Result<(), String> {
        let stream = TcpStream::connect((self.address.clone(), self.port))
            .await
            .map_err(|e| e.to_string())?;
        let (tx_input, mut rx_input) = mpsc::unbounded_channel();
        self.input = tx_input;
        let tx_output = self.output_dispatch.clone();
        self.handle = Some(tokio::spawn(async move {
            let mut stream = MudStream::new(stream, Default::default());
            loop {
                let input = tokio::select! {
                    input = rx_input.recv() => input,
                    fragments = stream.read() => match fragments? {
                        Some(fragments) => {
                            for fragment in fragments {
                                tx_output.send(fragment).map_err(|_| io::ErrorKind::BrokenPipe)?;
                            }
                            continue;
                        }
                        None => return Ok(()),
                    }
                };

                if let Some(input) = input {
                    stream.write_all(input.as_ref()).await?;
                }
            }
        }));
        Ok(())
    }

    pub fn disconnect(&self) -> bool {
        match &self.handle {
            Some(handle) if !handle.is_finished() => {
                handle.abort();
                true
            }
            _ => false,
        }
    }

    pub async fn get_output(&mut self) -> Result<ffi::OutputFragment, String> {
        match self.output.recv().await {
            Some(output) => Ok(output.into()),
            None => Err(io::ErrorKind::BrokenPipe.to_string()),
        }
    }

    pub fn send_input(&self, input: String) -> Result<(), String> {
        self.input.send(input).map_err(|e| e.to_string())
    }

    pub async fn wait_until_done(&mut self) -> Result<(), String> {
        let handle = match self.handle.take() {
            Some(handle) => handle,
            None => return Ok(()),
        };
        match handle.await {
            Ok(Ok(())) => Ok(()),
            Ok(Err(e)) => Err(e.to_string()),
            Err(e) => Err(e.to_string()),
        }
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
        pub fn $n(&self) -> bool {
            self.inner.flags.contains($v)
        }
    };
}

impl RustTextFragment {
    #[inline]
    fn text(&self) -> &[u8] {
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
