#![warn(unsafe_op_in_unsafe_fn)]
#![allow(clippy::result_large_err)]

extern crate enumeration;

#[macro_use]
mod in_place;

mod callback;
pub use callback::Callback;

mod constants;

mod plugins;
pub use plugins::{Pad, Plugin, PluginIndex, PluginMetadata, SendMatch, Sendable, Senders};

mod regex;
pub use regex::{Regex, RegexError};

mod send;
pub use send::{Alias, Event, Reaction, SendTo, Sender, Timer, Trigger, TriggerEffects};
