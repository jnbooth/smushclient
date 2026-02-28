#![allow(clippy::wrong_self_convention)]
use std::borrow::Cow;
use std::ffi::c_char;
use std::marker::PhantomData;
use std::slice;
use std::str::Utf8Error;

use cxx::{ExternType, type_id};

#[cxx::bridge]
mod ffi {
    #[namespace = "rust"]
    extern "C++" {
        include!("smushclient_qt/views.h");
        #[cxx_name = "string_view"]
        type StringView<'a>;
    }
}

/// Type used by the C++ side to send strings to the Rust side without needing to allocate memory.
/// This type cannot be constructed within Rust code.
#[repr(C)]
#[derive(Copy, Clone)]
pub struct StringView<'a> {
    data_: *const c_char,
    size_: usize,
    marker: PhantomData<&'a [c_char]>,
}

impl<'a> StringView<'a> {
    pub const fn as_slice(&self) -> &'a [u8] {
        if self.data_.is_null() {
            return &[];
        }
        // SAFETY: All safety requirements have been fulfilled. This type cannot be constructed from
        // Rust, so its lifetime cannot be manipulated.
        unsafe { slice::from_raw_parts(self.data_.cast(), self.size_) }
    }

    pub const fn is_empty(&self) -> bool {
        self.size_ == 0
    }

    pub const fn len(&self) -> usize {
        self.size_
    }

    pub fn to_string_lossy(&self) -> Cow<'a, str> {
        String::from_utf8_lossy(self.as_slice())
    }

    pub fn to_str(&self) -> Result<&'a str, Utf8Error> {
        str::from_utf8(self.as_slice())
    }

    pub fn to_vec(&self) -> Vec<u8> {
        self.as_slice().to_vec()
    }
}

impl AsRef<[u8]> for StringView<'_> {
    fn as_ref(&self) -> &[u8] {
        self.as_slice()
    }
}

impl<'a> From<StringView<'a>> for &'a [u8] {
    fn from(value: StringView<'a>) -> Self {
        value.as_slice()
    }
}

impl<'a> TryFrom<StringView<'a>> for &'a str {
    type Error = Utf8Error;

    #[inline]
    fn try_from(value: StringView<'a>) -> Result<Self, Self::Error> {
        str::from_utf8(value.as_slice())
    }
}

impl TryFrom<StringView<'_>> for String {
    type Error = Utf8Error;

    #[inline]
    fn try_from(value: StringView<'_>) -> Result<Self, Self::Error> {
        Ok(str::from_utf8(value.as_slice())?.to_owned())
    }
}

// SAFETY: Defined in smushclient-qt/cpp/bridge/views.h
unsafe impl ExternType for StringView<'_> {
    type Id = type_id!("rust::string_view");
    type Kind = cxx::kind::Trivial;
}
