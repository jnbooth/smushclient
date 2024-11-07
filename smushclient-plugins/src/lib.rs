#![warn(unsafe_op_in_unsafe_fn)]
#![allow(clippy::result_large_err)]

#[macro_use]
mod in_place;

mod constants;

mod plugin;
pub use plugin::{Plugin, PluginIndex, PluginLoadError, PluginMetadata};

mod regex;
pub use regex::{Regex, RegexError};

mod send;
pub use send::{
    Alias, Occurrence, Reaction, SendTarget, Sender, SenderLock, SenderLockError, Timer, Trigger,
};

pub use pcre2::bytes::{CaptureMatches, Captures, Match};
