#![warn(unsafe_op_in_unsafe_fn)]
#![allow(clippy::result_large_err)]

extern crate enumeration;

#[macro_use]
mod in_place;

mod callback;
pub use callback::Callback;

mod constants;

mod convert;
pub use convert::{ScriptArg, ScriptArgs, ScriptRes};

mod plugins;
pub use plugins::{
    LoadError, Pad, Plugin, PluginHandler, PluginIndex, PluginMetadata, PluginPack, SendMatch,
    Sendable, Senders, TriggerEffects,
};

mod regex;
pub use regex::{Regex, RegexError};

mod send;
pub use send::{Alias, Event, Reaction, SendTo, Sender, Timer, Trigger};
