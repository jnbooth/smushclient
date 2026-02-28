#![allow(
    clippy::elidable_lifetime_names,
    clippy::needless_lifetimes,
    clippy::needless_pass_by_value,
    clippy::unnecessary_box_returns
)]

mod api_code;
pub use api_code::ffi::ApiCode;

mod audio;

mod bytes_view;
pub use bytes_view::BytesView;

mod client;
pub use client::ffi::{
    AliasMenuItem, CommandSource, ExportKind, PluginPack, QAbstractSocket, ReplaceSenderResult,
    SmushClient, StyledSpan,
};

mod document;
pub use document::ffi::{
    Document, DynamicColor, NamedWildcard, SendScriptRequest, TelnetSource, TelnetVerb,
};

mod filter;

mod plugin_details;
pub use plugin_details::ffi::PluginDetails;

mod regex;
pub use regex::ffi::RegexParse;

mod sender;
pub use sender::AliasOutcomes;
pub use sender::ffi::{Alias, AliasOutcome, Occurrence, Timer, Trigger, UserSendTarget};

mod sender_map;
pub use sender_map::ffi::{SenderMap, SenderType};

mod send_request;
pub use send_request::ffi::{SendRequest, SendTarget, SendTimer, SenderKind};

pub mod spans;

mod timekeeper;
pub use timekeeper::ffi::Timekeeper;

mod util;

mod world;
pub use world::ffi::{
    AutoConnect, LogFormat, LogMode, MXPDebugLevel, ScriptRecompile, UseMxp, World,
};

mod string_view;
pub use string_view::StringView;

mod variable_view;
pub use variable_view::VariableView;
