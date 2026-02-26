use std::error::Error;
use std::fmt;
use std::num::TryFromIntError;

use cxx_qt_lib::{QByteArray, QColor, QVariant};
use mud_transformer::mxp::RgbColor;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct OutOfRangeError;

impl fmt::Display for OutOfRangeError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.write_str("out of range")
    }
}

impl Error for OutOfRangeError {}

impl From<TryFromIntError> for OutOfRangeError {
    fn from(_value: TryFromIntError) -> Self {
        Self
    }
}

macro_rules! impl_deref {
    ($t:ty, $target:ty, $field:ident) => {
        impl std::ops::Deref for $t {
            type Target = $target;

            fn deref(&self) -> &Self::Target {
                &self.$field
            }
        }

        impl std::ops::DerefMut for $t {
            fn deref_mut(&mut self) -> &mut Self::Target {
                &mut self.$field
            }
        }
    };
}
pub(crate) use impl_deref;

macro_rules! impl_convert_enum {
    ($ffi:ty, $rust:path, $($variant:ident),+ $(,)?) => {
        impl From<$rust> for $ffi {
            fn from(value: $rust) -> Self {
                match value {
                    $(<$rust>::$variant => Self::$variant),+
                }
            }
        }
        impl TryFrom<$ffi> for $rust {
            type Error = crate::convert::OutOfRangeError;

            fn try_from(value: $ffi) -> Result<Self, crate::convert::OutOfRangeError> {
                match value {
                    $(<$ffi>::$variant => Ok(Self::$variant)),+,
                    _ => Err(crate::convert::OutOfRangeError)
                }
            }
        }
    }
}
pub(crate) use impl_convert_enum;

macro_rules! impl_constructor {
    (<$($l:lifetime),*>, $t:ty, $a:ty, { $i:item }) => {
        impl <$($l),*> cxx_qt::Constructor<$a> for $t {
            type BaseArguments = ();
            type InitializeArguments = ();
            type NewArguments = $a;

            fn route_arguments(
                args: Self::NewArguments,
            ) -> (
                Self::NewArguments,
                Self::BaseArguments,
                Self::InitializeArguments,
            ) {
                (args, (), ())
            }

            $i
        }
    };
}
pub(crate) use impl_constructor;
use smushclient::OptionValue;

pub trait Convert<T> {
    fn convert(&self) -> T;
}

impl Convert<RgbColor> for QColor {
    fn convert(&self) -> RgbColor {
        let rgb = self.to_rgb();
        #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
        RgbColor {
            r: rgb.red() as u8,
            g: rgb.green() as u8,
            b: rgb.blue() as u8,
        }
    }
}

impl Convert<Option<RgbColor>> for QColor {
    fn convert(&self) -> Option<RgbColor> {
        if self.alpha() == 0 || !self.is_valid() {
            return None;
        }
        Some(self.convert())
    }
}

impl Convert<QColor> for RgbColor {
    fn convert(&self) -> QColor {
        QColor::from_rgb(self.r.into(), self.g.into(), self.b.into())
    }
}

impl Convert<QColor> for Option<RgbColor> {
    fn convert(&self) -> QColor {
        match self {
            Some(color) => color.convert(),
            None => QColor::from_rgba(0, 0, 0, 0),
        }
    }
}

impl Convert<QVariant> for OptionValue<'_> {
    fn convert(&self) -> QVariant {
        match self {
            Self::Null => QVariant::default(),
            Self::Alpha(s) => QVariant::from(&QByteArray::from(*s)),
            Self::Borrow(s) => QVariant::from(&QByteArray::from(&**s)),
            Self::Color(color) => QVariant::from(&QByteArray::from(&color.to_string())),
            Self::Numeric(i) => QVariant::from(i),
        }
    }
}
