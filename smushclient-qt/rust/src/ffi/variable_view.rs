use std::cell::Ref;
use std::ffi::c_char;
use std::ptr;

use cxx::{ExternType, type_id};
use smushclient::LuaStr;

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("variableview.h");
        type VariableView;
    }
}

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
        if value.is_empty() {
            return Self::EMPTY;
        }
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

impl From<Option<Ref<'_, LuaStr>>> for VariableView {
    fn from(value: Option<Ref<'_, LuaStr>>) -> Self {
        match value {
            Some(value) => Self::from(&*value),
            None => Self::EMPTY,
        }
    }
}

// SAFETY: Defined in smushclient-qt/cpp/bridge/variableview.h
unsafe impl ExternType for VariableView {
    type Id = type_id!("VariableView");
    type Kind = cxx::kind::Trivial;
}
