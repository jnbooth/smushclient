#![allow(clippy::struct_field_names, clippy::wildcard_imports)]

mod v1;
mod v2;
mod v3;
mod v4;

use serde::de::DeserializeOwned;
pub(crate) use v1::World as V1;
pub(crate) use v2::World as V2;
pub(crate) use v3::World as V3;
pub(crate) use v4::World as V4;

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
