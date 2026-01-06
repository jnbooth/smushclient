#![allow(clippy::needless_lifetimes)]
#![allow(clippy::needless_pass_by_value)]
#![allow(clippy::unnecessary_box_returns)]

mod api_code;
pub use api_code::ffi::ApiCode;

mod audio;

mod client;
pub use client::ffi::{
    AliasBool, CommandSource, PluginPack, QAbstractSocket, ReplaceSenderResult, SmushClient,
    TimerBool, TriggerBool,
};

mod document;
pub use document::ffi::{
    Document, NamedWildcard, SendRequest, SendScriptRequest, SendTo, TelnetSource, TelnetVerb,
};

mod plugin_details;
pub use plugin_details::ffi::PluginDetails;

mod sender;
pub use sender::AliasOutcomes;
pub use sender::ffi::{
    Alias, AliasOutcome, Occurrence, SendTarget, Timer, Trigger, UserSendTarget,
};

mod sender_map;
pub use sender_map::ffi::{SenderMap, SenderType};

mod timekeeper;
pub use timekeeper::ffi::{SendTimer, Timekeeper};

mod util;

mod world;
pub use world::ffi::{
    AutoConnect, LogFormat, LogMode, MXPDebugLevel, ScriptRecompile, UseMxp, World,
};

mod variable_view;
pub use variable_view::VariableView;
