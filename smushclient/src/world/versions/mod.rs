#![allow(clippy::wildcard_imports)]
#![allow(clippy::struct_field_names)]

use std::io::Read;

use serde::de::DeserializeOwned;

mod v1;
pub use v1::World as V1;

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
