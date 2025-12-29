#![allow(clippy::wildcard_imports)]
#![allow(clippy::struct_field_names)]

mod v1;

use serde::de::DeserializeOwned;
pub(crate) use v1::World as V1;

pub trait Migrate {
    fn migrate(bytes: &[u8]) -> Result<super::World, super::PersistError>;
}

impl<T: DeserializeOwned + Into<super::World>> Migrate for T {
    fn migrate(bytes: &[u8]) -> Result<super::World, super::PersistError> {
        match postcard::from_bytes::<Self>(bytes) {
            Ok(world) => Ok(world.into()),
            Err(err) => Err(err.into()),
        }
    }
}
