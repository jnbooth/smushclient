#![allow(clippy::unnecessary_cast)]

#[macro_use]
mod convert;

mod bridge;
mod client;
mod enums;
mod error;
mod output;
mod sync;
mod world;

use bridge::{RustMudBridge, RustOutputStream};
use enums::ffi::{EffectFragment, SendTo, TelnetFragment};
use mud_transformer::OutputFragment;
use output::{RustMxpLink, RustTextFragment};
use world::ffi::{RgbColor, SendRequest, World};
use world::{create_world, read_world, write_world};

fn convert_world(world: Result<smushclient::World, String>) -> Result<World, String> {
    world.map(Into::into)
}

#[swift_bridge::bridge]
mod ffi {
    #[swift_bridge(already_declared, swift_repr = "struct")]
    struct RgbColor;
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
        fn foreground(&self) -> RgbColor;
        #[swift_bridge(return_into)]
        fn background(&self) -> RgbColor;
        #[swift_bridge(swift_name = "isBlink")]
        fn is_blink(&self) -> bool;
        #[swift_bridge(swift_name = "isBold")]
        fn is_bold(&self) -> bool;
        #[swift_bridge(swift_name = "isHighlight")]
        fn is_highlight(&self) -> bool;
        #[swift_bridge(swift_name = "isInverse")]
        fn is_inverse(&self) -> bool;
        #[swift_bridge(swift_name = "isItalic")]
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
        MxpError(String),
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
        #[swift_bridge(init, args_into = (world))]
        fn new(world: World) -> RustMudBridge;
        #[swift_bridge(associated_to = RustMudBridge)]
        fn load(path: String) -> Result<RustMudBridge, String>;
        fn save(&self, path: String) -> Result<(), String>;
        #[swift_bridge(return_into)]
        fn world(&self) -> World;
        #[swift_bridge(args_into = (world))]
        fn set_world(&mut self, world: World);
        fn connected(&self) -> bool;
        async fn connect(&mut self) -> Result<(), String>;
        async fn disconnect(&mut self) -> Result<(), String>;
        async fn receive(&mut self) -> Result<RustOutputStream, String>;
        async fn send(&mut self, input: String) -> Result<(), String>;
    }

    extern "Rust" {
        #[swift_bridge(return_into)]
        fn create_world() -> World;
        #[swift_bridge(return_with = convert_world)]
        fn read_world(data: &[u8]) -> Result<World, String>;
        #[swift_bridge(args_into = (world))]
        fn write_world(world: World) -> Result<Vec<u8>, String>;
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
            OutputFragment::MxpError(error) => Self::MxpError(error.to_string()),
            OutputFragment::PageBreak => Self::PageBreak,
            OutputFragment::Telnet(telnet) => Self::Telnet(telnet.into()),
            OutputFragment::Text(text) => Self::Text(text.into()),
        }
    }
}
