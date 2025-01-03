#![warn(unsafe_op_in_unsafe_fn)]

#[macro_use]
mod in_place;

mod constants;

mod cursor_vec;
pub use cursor_vec::CursorVec;

mod error;
pub use error::LoadError;

mod plugin;
pub use plugin::{Plugin, PluginIndex, PluginLoadError, PluginMetadata};

mod regex;
pub use regex::{Regex, RegexError};

mod send;
pub use send::{Alias, Occurrence, Reaction, SendTarget, Sender, Timer, Trigger};

pub use pcre2::bytes::{CaptureMatches, Captures, Match};

pub use quick_xml::DeError as XmlError;
