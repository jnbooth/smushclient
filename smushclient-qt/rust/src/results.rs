use crate::bridge::AliasOutcomes;
use crate::ffi;
use smushclient::{AliasOutcome, SendIterable, SenderAccessError};

const fn flag_if(flag: ffi::AliasOutcome, pred: bool) -> u8 {
    if pred {
        flag.repr
    } else {
        0
    }
}

pub const fn convert_alias_outcome(outcome: AliasOutcome) -> AliasOutcomes {
    AliasOutcomes(
        flag_if(ffi::AliasOutcome::Display, outcome.display)
            | flag_if(ffi::AliasOutcome::Remember, outcome.remember)
            | flag_if(ffi::AliasOutcome::Send, outcome.send),
    )
}

pub trait IntoResultCode {
    fn code(self) -> i32;
}

impl IntoResultCode for SenderAccessError {
    fn code(self) -> i32 {
        match self {
            SenderAccessError::NotFound => ffi::SenderAccessResult::NotFound.repr,
            SenderAccessError::Locked => ffi::SenderAccessResult::Locked.repr,
            SenderAccessError::LabelConflict(pos) => {
                ffi::SenderAccessResult::LabelConflict.repr - i32::try_from(pos).unwrap_or(i32::MAX)
            }
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

impl<T: SendIterable> IntoErrorCode for Result<T, SenderAccessError> {
    fn code(self) -> i32 {
        match self {
            Ok(_) => ffi::SenderAccessResult::Ok.repr,
            Err(e) => e.code(),
        }
    }
}
