pub type PluginIndex = usize;

mod matching;
pub use matching::{Matches, SendMatch, SendMatches};

mod pad;
pub use pad::{Pad, PadSource};

mod plugin;
pub use plugin::{Plugin, PluginLoadError, PluginMetadata};
