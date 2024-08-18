#![allow(clippy::expl_impl_clone_on_copy)]
#![allow(clippy::ptr_as_ptr)]
#![allow(clippy::unnecessary_cast)]

#[macro_use]
extern crate enumeration;

#[macro_use]
mod convert;

mod bindings;
mod bridge;
mod client;
mod error;
mod impls;
mod io;
mod output;
mod shared;
mod sync;
