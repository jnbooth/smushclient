use std::io;

use smushclient::{AudioError, SenderAccessError};
use smushclient_plugins::{Alias, Timer, Trigger};

use crate::ffi;

pub trait IntoCode {
    fn code(self) -> i32;
}

impl IntoCode for Result<usize, ffi::ReplaceSenderResult> {
    fn code(self) -> i32 {
        match self {
            #[allow(clippy::cast_possible_truncation)]
            #[allow(clippy::cast_possible_wrap)]
            Self::Ok(index) => index as i32,
            Self::Err(e) => e.repr,
        }
    }
}

impl From<SenderAccessError> for ffi::ReplaceSenderResult {
    fn from(value: SenderAccessError) -> Self {
        match value {
            SenderAccessError::LabelConflict(_) => Self::Conflict,
            SenderAccessError::NotFound => Self::NotFound,
            SenderAccessError::Unchanged => Self::Unchanged,
        }
    }
}

pub trait SenderAccessCode {
    fn code(error: SenderAccessError) -> ffi::ApiCode;
}

impl SenderAccessCode for Alias {
    fn code(error: SenderAccessError) -> ffi::ApiCode {
        match error {
            SenderAccessError::NotFound => ffi::ApiCode::AliasNotFound,
            SenderAccessError::LabelConflict(_) => ffi::ApiCode::AliasAlreadyExists,
            SenderAccessError::Unchanged => ffi::ApiCode::OK,
        }
    }
}

impl SenderAccessCode for Timer {
    fn code(error: SenderAccessError) -> ffi::ApiCode {
        match error {
            SenderAccessError::NotFound => ffi::ApiCode::TimerNotFound,
            SenderAccessError::LabelConflict(_) => ffi::ApiCode::TimerAlreadyExists,
            SenderAccessError::Unchanged => ffi::ApiCode::OK,
        }
    }
}

impl SenderAccessCode for Trigger {
    fn code(error: SenderAccessError) -> ffi::ApiCode {
        match error {
            SenderAccessError::NotFound => ffi::ApiCode::TriggerNotFound,
            SenderAccessError::LabelConflict(_) => ffi::ApiCode::TriggerAlreadyExists,
            SenderAccessError::Unchanged => ffi::ApiCode::OK,
        }
    }
}

pub trait IntoSenderAccessCode {
    fn code<T: SenderAccessCode>(self) -> ffi::ApiCode;
}

impl<R> IntoSenderAccessCode for Result<R, SenderAccessError> {
    fn code<T: SenderAccessCode>(self) -> ffi::ApiCode {
        match self {
            Self::Ok(_) => ffi::ApiCode::OK,
            Self::Err(e) => T::code(e),
        }
    }
}

impl From<Result<(), AudioError>> for ffi::ApiCode {
    fn from(value: Result<(), AudioError>) -> Self {
        match value {
            Ok(()) => Self::OK,
            Err(AudioError::FileError(error)) if error.kind() == io::ErrorKind::NotFound => {
                Self::FileNotFound
            }
            Err(AudioError::SinkOutOfRange) => Self::BadParameter,
            _ => Self::CannotPlaySound,
        }
    }
}
