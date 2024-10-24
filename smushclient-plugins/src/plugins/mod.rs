pub type PluginIndex = usize;

mod matching;
pub use matching::{SendMatch, SendMatchIter, SendMatchIterable};

mod pad;
pub use pad::{Pad, PadSource};

mod plugin;
pub use plugin::{Plugin, PluginLoadError, PluginMetadata};
