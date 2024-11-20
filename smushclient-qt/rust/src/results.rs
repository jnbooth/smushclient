use crate::ffi;
use smushclient::{SendIterable, SenderAccessError};

pub trait IntoResultCode {
    fn code(self) -> i32;
}

impl IntoResultCode for SenderAccessError {
    fn code(self) -> i32 {
        match self {
            Self::NotFound => ffi::SenderAccessResult::NotFound.repr,
            Self::LabelConflict(pos) => {
                ffi::SenderAccessResult::LabelConflict.repr - i32::try_from(pos).unwrap_or(i32::MAX)
            }
            Self::Unchanged => ffi::SenderAccessResult::Unchanged.repr,
        }
    }
}

impl IntoResultCode for Result<usize, SenderAccessError> {
    fn code(self) -> i32 {
        match self {
            Ok(val) => i32::try_from(val).unwrap_or(i32::MAX),
            Err(e) => e.code(),
        }
    }
}

impl<T> IntoResultCode for Result<(usize, T), SenderAccessError> {
    fn code(self) -> i32 {
        match self {
            Ok((val, _)) => i32::try_from(val).unwrap_or(i32::MAX),
            Err(e) => e.code(),
        }
    }
}

impl IntoResultCode for Result<(), SenderAccessError> {
    fn code(self) -> i32 {
        match self {
            Ok(()) => ffi::SenderAccessResult::Ok.repr,
            Err(e) => e.code(),
        }
    }
}

pub trait IntoErrorCode {
    fn code(self) -> i32;
}

impl<T: SendIterable> IntoErrorCode for Result<&T, SenderAccessError> {
    fn code(self) -> i32 {
        match self {
            Ok(_) => ffi::SenderAccessResult::Ok.repr,
            Err(e) => e.code(),
        }
    }
}
