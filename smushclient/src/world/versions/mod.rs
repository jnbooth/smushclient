#![allow(clippy::wildcard_imports)]
#![allow(clippy::struct_field_names)]

use std::io::Read;

use serde::de::DeserializeOwned;

mod v1;
pub use v1::World as V1;

mod v2;
pub use v2::World as V2;

mod v3;
pub use v3::World as V3;

mod v4;
pub use v4::World as V4;

mod v5;
pub use v5::World as V5;

pub trait WorldVersion {
    fn migrate(reader: &mut dyn Read) -> Result<super::World, super::PersistError>;
}

impl<T> WorldVersion for T
where
    T: DeserializeOwned,
    super::World: From<T>,
{
    fn migrate(reader: &mut dyn Read) -> Result<super::World, super::PersistError> {
        match bincode::deserialize_from(reader) {
            Ok(world) => Ok(super::World::from(world)),
            Err(e) => Err(e.into()),
        }
    }
}
