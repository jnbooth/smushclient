use std::cell::Ref;
use std::ffi::c_char;
use std::ptr;

use cxx::{ExternType, type_id};
use smushclient::LuaStr;

#[cxx::bridge]
mod ffi {
    #[namespace = "rust"]
    extern "C++" {
        include!("smushclient_qt/views.h");
        #[cxx_name = "variable_view"]
        type VariableView;
    }
}

/// Type used by the Rust side to send world variables to the C++ side without needing to allocate
/// memory. This type cannot be constructed within C++ code. Furthermore, its data cannot be
/// accessed on the Rust side, as doing so could potentially be unsafe.
///
/// If no variable matching the key is found, the `VariableView`'s data pointer will be null.
#[repr(C)]
pub struct VariableView {
    data_: *const c_char,
    size_: usize,
}

impl VariableView {
    const EMPTY: Self = Self {
        data_: ptr::null(),
        size_: 0,
    };
}

impl From<&[u8]> for VariableView {
    fn from(value: &[u8]) -> Self {
        Self {
            data_: value.as_ptr().cast(),
            size_: value.len(),
        }
    }
}

impl From<&str> for VariableView {
    fn from(value: &str) -> Self {
        Self::from(value.as_bytes())
    }
}

impl From<Ref<'_, LuaStr>> for VariableView {
    fn from(value: Ref<'_, LuaStr>) -> Self {
        Self::from(&*value)
    }
}

impl<T> From<Option<T>> for VariableView
where
    VariableView: From<T>,
{
    fn from(value: Option<T>) -> Self {
        match value {
            Some(value) => Self::from(value),
            None => Self::EMPTY,
        }
    }
}

// SAFETY: Defined in smushclient-qt/cpp/bridge/views.h
unsafe impl ExternType for VariableView {
    type Id = type_id!("rust::variable_view");
    type Kind = cxx::kind::Trivial;
}
