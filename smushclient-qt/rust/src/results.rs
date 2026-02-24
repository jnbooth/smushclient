use std::io;

use smushclient::world::SetOptionError;
use smushclient::{AudioError, OptionError, SenderAccessError};
use smushclient_plugins::{Alias, Timer, Trigger};

use crate::ffi::{self, ApiCode};

pub trait IntoCode {
    fn code(self) -> i32;
}

impl IntoCode for Result<usize, ffi::ReplaceSenderResult> {
    fn code(self) -> i32 {
        match self {
            #[allow(clippy::cast_possible_truncation, clippy::cast_possible_wrap)]
            Self::Ok(index) => index as i32,
            Self::Err(e) => e.repr,
        }
    }
}

impl From<SenderAccessError> for ffi::ReplaceSenderResult {
    fn from(value: SenderAccessError) -> Self {
        match value {
            SenderAccessError::LabelConflict(_) | SenderAccessError::ItemInUse => Self::Conflict,
            SenderAccessError::NotFound => Self::NotFound,
            SenderAccessError::Unchanged => Self::Unchanged,
        }
    }
}

pub trait SenderAccessCode {
    fn code(error: SenderAccessError) -> ApiCode;
}

impl SenderAccessCode for Alias {
    fn code(error: SenderAccessError) -> ApiCode {
        match error {
            SenderAccessError::NotFound => ApiCode::AliasNotFound,
            SenderAccessError::LabelConflict(_) => ApiCode::AliasAlreadyExists,
            SenderAccessError::ItemInUse => ApiCode::ItemInUse,
            SenderAccessError::Unchanged => ApiCode::OK,
        }
    }
}

impl SenderAccessCode for Timer {
    fn code(error: SenderAccessError) -> ApiCode {
        match error {
            SenderAccessError::NotFound => ApiCode::TimerNotFound,
            SenderAccessError::LabelConflict(_) => ApiCode::TimerAlreadyExists,
            SenderAccessError::ItemInUse => ApiCode::ItemInUse,
            SenderAccessError::Unchanged => ApiCode::OK,
        }
    }
}

impl SenderAccessCode for Trigger {
    fn code(error: SenderAccessError) -> ApiCode {
        match error {
            SenderAccessError::NotFound => ApiCode::TriggerNotFound,
            SenderAccessError::LabelConflict(_) => ApiCode::TriggerAlreadyExists,
            SenderAccessError::ItemInUse => ApiCode::ItemInUse,
            SenderAccessError::Unchanged => ApiCode::OK,
        }
    }
}

pub trait IntoSenderAccessCode {
    fn code<T: SenderAccessCode>(self) -> ApiCode;
}

impl<R> IntoSenderAccessCode for Result<R, SenderAccessError> {
    fn code<T: SenderAccessCode>(self) -> ApiCode {
        match self {
            Self::Ok(_) => ApiCode::OK,
            Self::Err(e) => T::code(e),
        }
    }
}

impl From<AudioError> for ApiCode {
    fn from(value: AudioError) -> Self {
        match value {
            AudioError::FileError(error) if error.kind() == io::ErrorKind::NotFound => {
                Self::FileNotFound
            }
            AudioError::SinkOutOfRange => Self::BadParameter,
            _ => Self::CannotPlaySound,
        }
    }
}

impl From<OptionError> for ApiCode {
    fn from(value: OptionError) -> Self {
        match value {
            OptionError::InvalidObjectLabel => Self::InvalidObjectLabel,
            OptionError::AliasCannotBeEmpty => Self::AliasCannotBeEmpty,
            OptionError::TriggerCannotBeEmpty => Self::TriggerCannotBeEmpty,
            OptionError::ScriptNameNotLocated => Self::ScriptNameNotLocated,
            OptionError::BadRegularExpression => Self::BadRegularExpression,
            OptionError::PluginCannotSetOption => Self::PluginCannotSetOption,
            OptionError::OptionOutOfRange => Self::OptionOutOfRange,
            OptionError::UnknownOption => Self::UnknownOption,
        }
    }
}

impl From<SetOptionError> for ApiCode {
    fn from(value: SetOptionError) -> Self {
        match value {
            SetOptionError::OptionOutOfRange => Self::OptionOutOfRange,
            SetOptionError::UnknownOption => Self::UnknownOption,
            SetOptionError::PluginCannotSetOption => Self::PluginCannotSetOption,
        }
    }
}

pub trait IntoApiCode {
    fn code(self) -> ApiCode;
}

impl<E: Into<ApiCode>> IntoApiCode for Result<(), E> {
    fn code(self) -> ApiCode {
        match self {
            Ok(()) => ApiCode::OK,
            Err(e) => e.into(),
        }
    }
}
