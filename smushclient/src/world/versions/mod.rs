#![allow(clippy::struct_field_names, clippy::wildcard_imports)]

mod v1;
mod v2;
mod v3;
mod v4;
mod v5;
mod v6;

use serde::de::DeserializeOwned;
pub(crate) use v1::World as V1;
pub(crate) use v2::World as V2;
pub(crate) use v3::World as V3;
pub(crate) use v4::World as V4;
pub(crate) use v5::World as V5;
pub(crate) use v6::World as V6;

pub trait Migrate {
    fn migrate(bytes: &[u8]) -> postcard::Result<super::World<'static>>;
}

impl<T> Migrate for T
where
    T: DeserializeOwned,
    super::World<'static>: From<T>,
{
    fn migrate(bytes: &[u8]) -> postcard::Result<super::World<'static>> {
        postcard::from_bytes::<Self>(bytes).map(Into::into)
    }
}
