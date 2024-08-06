#![allow(clippy::unnecessary_cast)]

#[macro_use]
mod convert;

mod bridge;
mod client;
mod enums;
mod output;
mod sync;
mod world;

use bridge::{RustMudBridge, RustOutputStream};
use enums::ffi::{EffectFragment, SendTo, TelnetFragment};
use mud_transformer::OutputFragment;
use output::{RustMxpLink, RustTextFragment};
use world::ffi::{MudColor, SendRequest};

#[swift_bridge::bridge]
mod ffi {
    #[swift_bridge(already_declared)]
    enum MudColor {}
    #[swift_bridge(already_declared)]
    enum SendTo {}
    #[swift_bridge(already_declared)]
    enum EffectFragment {}
    #[swift_bridge(already_declared)]
    enum TelnetFragment {}
    #[swift_bridge(already_declared, swift_repr = "struct")]
    struct SendRequest;
    #[swift_bridge(already_declared, swift_repr = "struct")]
    struct World;

    extern "Rust" {
        type RustMxpLink;
        fn action(&self) -> &str;
        fn hint(&self) -> Option<&str>;
        fn prompts(&self) -> Vec<String>;
        #[swift_bridge(return_into)]
        fn sendto(&self) -> SendTo;
    }

    extern "Rust" {
        type RustTextFragment;
        fn text(&self) -> &str;
        #[swift_bridge(return_into)]
        fn foreground(&self) -> MudColor;
        #[swift_bridge(return_into)]
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

    enum OutputFragment {
        Effect(EffectFragment),
        Hr,
        Image(String),
        LineBreak,
        PageBreak,
        Send(SendRequest),
        Sound(String),
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

pub use ffi::OutputFragment as FfiOutputFragment;

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
