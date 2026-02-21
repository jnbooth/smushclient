#![allow(clippy::wrong_self_convention)]
use std::ffi::c_char;
use std::marker::PhantomData;
use std::slice;

use cxx::{ExternType, type_id};

#[cxx::bridge]
mod ffi {
    #[namespace = "rust"]
    extern "C++" {
        include!("smushclient_qt/views.h");
        #[cxx_name = "bytes_view"]
        type BytesView<'a>;
    }
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct BytesView<'a> {
    data_: *const c_char,
    size_: usize,
    marker: PhantomData<&'a [c_char]>,
}

impl<'a> BytesView<'a> {
    pub const fn len(&self) -> usize {
        self.size_
    }

    pub const fn is_empty(&self) -> bool {
        self.size_ == 0
    }

    pub const fn as_slice(&self) -> &'a [u8] {
        if self.data_.is_null() {
            return &[];
        }
        // SAFETY: All safety requirements have been fulfilled because this type cannot be
        // constructed from Rust.
        unsafe { slice::from_raw_parts(self.data_.cast(), self.size_) }
    }

    pub fn to_vec(&self) -> Vec<u8> {
        self.as_slice().to_vec()
    }
}

impl AsRef<[u8]> for BytesView<'_> {
    fn as_ref(&self) -> &[u8] {
        self.as_slice()
    }
}

impl<'a> From<BytesView<'a>> for &'a [u8] {
    fn from(value: BytesView<'a>) -> Self {
        value.as_slice()
    }
}

// SAFETY: Defined in smushclient-qt/cpp/bridge/views.h
unsafe impl ExternType for BytesView<'_> {
    type Id = type_id!("rust::bytes_view");
    type Kind = cxx::kind::Trivial;
}
