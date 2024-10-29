pub type PluginIndex = usize;

mod pad;
pub use pad::{Pad, PadSource};

mod plugin;
pub use plugin::{Plugin, PluginLoadError, PluginMetadata};
