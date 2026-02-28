#![allow(clippy::match_same_arms)]
mod alias;

mod color;

mod decode;

mod encode;
pub use encode::OptionValue;

mod error;
pub use error::{OptionError, SetOptionError};

mod optionable;
pub use optionable::Optionable;

mod sender;

mod reaction;

mod timer;

mod trigger;

mod world;
pub use world::OptionCaller;
