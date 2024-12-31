#![allow(clippy::float_cmp)]
#![allow(clippy::needless_pass_by_value)]

#[macro_use]
mod convert;

mod adapter;
mod bridge;
use bridge::ffi;
mod client;
mod client_ffi;
mod colors;
mod get_info;
mod handler;
mod impls;
mod modeled;
mod results;
mod sender;
mod sync;
mod world;
