use crate::bridge::{RustMudBridge, RustOutputStream};
use crate::convert::Convert;
use crate::error::UnsupportedError;
use crate::io::{create_world, read_world, write_world};
use crate::output::{RustMxpLink, RustTextFragment};
use crate::shared::ffi::{EffectFragment, RgbColor, SendRequest, SendTo, TelnetFragment, World};
use mud_transformer::mxp::Heading;
use mud_transformer::{EntityFragment, OutputFragment};

fn convert_opt<Ffi, Rust: Into<Ffi>>(from: Option<Rust>) -> Option<Ffi> {
    from.map(Into::into)
}

fn convert_result<Ffi, Rust: Into<Ffi>>(from: Result<Rust, String>) -> Result<Ffi, String> {
    from.map(Into::into)
}

fn convert_world<T: Into<World>>(from: Result<T, String>) -> Result<World, String> {
    convert_result(from)
}

#[swift_bridge::bridge]
pub mod ffi {
    #[swift_bridge(already_declared, swift_repr = "struct")]
    struct RgbColor;
    #[swift_bridge(already_declared)]
    enum EffectFragment {}
    #[swift_bridge(already_declared)]
    enum TelnetFragment {}
    #[swift_bridge(already_declared, swift_repr = "struct")]
    struct SendRequest;
    #[swift_bridge(already_declared, swift_repr = "struct")]
    struct World;
    #[swift_bridge(already_declared)]
    enum SendTo {}

    enum Heading {
        H1,
        H2,
        H3,
        H4,
        H5,
        H6,
    }

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
        #[swift_bridge(swift_name = "isItalic")]
        fn is_italic(&self) -> bool;
        #[swift_bridge(swift_name = "isStrikeout")]
        fn is_strikeout(&self) -> bool;
        #[swift_bridge(swift_name = "isUnderline")]
        fn is_underline(&self) -> bool;
        fn link(&self) -> Option<&RustMxpLink>;
        #[swift_bridge(return_with = convert_opt)]
        fn heading(&self) -> Option<Heading>;
    }

    enum OutputFragment {
        Effect(EffectFragment),
        Hr,
        LineBreak,
        MxpError(String),
        MxpEntitySet {
            name: String,
            value: String,
            publish: bool,
            is_variable: bool,
        },
        MxpEntityUnset {
            name: String,
            is_variable: bool,
        },
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
        #[swift_bridge(swift_name = "createWorld", return_into)]
        fn create_world() -> World;
        #[swift_bridge(swift_name = "readWorld", return_with = convert_world)]
        fn read_world(data: &[u8]) -> Result<World, String>;
        #[swift_bridge(swift_name = "writeWorld", args_into = (world))]
        fn write_world(world: World) -> Result<Vec<u8>, String>;
    }
}

impl_convert_enum!(ffi::Heading, Heading, H1, H2, H3, H4, H5, H6);

impl TryFrom<OutputFragment> for ffi::OutputFragment {
    type Error = UnsupportedError;

    fn try_from(value: OutputFragment) -> Result<Self, Self::Error> {
        Ok(match value {
            OutputFragment::Effect(effect) => Self::Effect(effect.try_into()?),
            OutputFragment::Frame(_) => return Err(UnsupportedError("<frame>")),
            OutputFragment::Hr => Self::Hr,
            OutputFragment::Image(_) => return Err(UnsupportedError("<image>")),
            OutputFragment::LineBreak => Self::LineBreak,
            OutputFragment::MxpEntity(EntityFragment::Set {
                name,
                value,
                publish,
                is_variable,
            }) => Self::MxpEntitySet {
                name,
                value,
                publish,
                is_variable,
            },
            OutputFragment::MxpEntity(EntityFragment::Unset { name, is_variable }) => {
                Self::MxpEntityUnset { name, is_variable }
            }
            OutputFragment::MxpError(error) => Self::MxpError(error.to_string()),
            OutputFragment::PageBreak => Self::PageBreak,
            OutputFragment::Telnet(telnet) => Self::Telnet(telnet.into()),
            OutputFragment::Text(text) => Self::Text(text.into()),
        })
    }
}
