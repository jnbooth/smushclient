#![cfg_attr(test, allow(clippy::unwrap_used))]
mod in_place;

mod constants;

mod cursor_vec;
pub use cursor_vec::CursorVec;

mod error;
pub use error::LoadError;

mod plugin;
pub use plugin::{Plugin, PluginIndex, PluginLoadError, PluginMetadata};

mod regex;
pub use regex::{CaptureMatches, Captures, Match, Regex, RegexError};

mod send;
pub use quick_xml::{DeError as XmlError, SeError as XmlSerError};
pub use send::{Alias, Occurrence, Reaction, SendTarget, Sender, Timer, Trigger};
