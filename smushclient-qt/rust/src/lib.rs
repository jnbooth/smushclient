#![allow(clippy::float_cmp)]
#![allow(clippy::needless_pass_by_value)]

#[macro_use]
extern crate enumeration;

#[macro_use]
mod convert;

mod adapter;
mod bridge;
use bridge::ffi;
mod client;
mod colors;
mod get_info;
mod handler;
mod impls;
mod results;
mod sender;
mod sync;
mod world;
