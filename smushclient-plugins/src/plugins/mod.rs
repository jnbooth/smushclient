pub type PluginIndex = usize;

mod matching;
pub use matching::{Matches, SendMatch};

mod pad;
pub use pad::Pad;

mod plugin;
pub use plugin::{Plugin, PluginLoadError, PluginMetadata};
