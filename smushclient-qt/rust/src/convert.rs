#![allow(clippy::cast_possible_truncation)]
#![allow(clippy::cast_possible_wrap)]
#![allow(clippy::cast_sign_loss)]
use std::error::Error;
use std::ffi::OsStr;
use std::fmt;
use std::num::TryFromIntError;
use std::path::PathBuf;

use cxx::ExternType;
use cxx::kind::Trivial;
use cxx_qt_lib::{QByteArray, QColor, QList, QString, QStringList, QVector};
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

            fn try_from(value: $ffi) -> Result<Self, Self::Error> {
                match value {
                    $(<$ffi>::$variant => Ok(Self::$variant)),+,
                    _ => Err(crate::convert::OutOfRangeError)
                }
            }
        }
    }
}
pub(crate) use impl_convert_enum;

macro_rules! impl_convert_enum_opt {
    ($ffi:ty, $rust:path, $fallback:ident, $($variant:ident),+ $(,)?) => {
        impl From<$rust> for $ffi {
            fn from(value: $rust) -> Self {
                match value {
                    $(<$rust>::$variant => Self::$variant),+
                }
            }
        }
        impl From<Option<$rust>> for $ffi {
            fn from(value: Option<$rust>) -> Self {
                match value {
                    None => Self::$fallback,
                    Some(value) => <$ffi>::from(value),
                }
            }
        }
        impl TryFrom<$ffi> for Option<$rust> {
            type Error = crate::convert::OutOfRangeError;

            fn try_from(value: $ffi) -> Result<Self, Self::Error> {
                match value {
                    <$ffi>::$fallback => Ok(None),
                    $(<$ffi>::$variant => Ok(Some(<$rust>::$variant))),+,
                    _ => Err(crate::convert::OutOfRangeError),
                }
            }
        }
        impl Default for $ffi {
            fn default() -> Self {
                Self::$fallback
            }
        }
    }
}
pub(crate) use impl_convert_enum_opt;

macro_rules! impl_constructor {
    ($t:ty, $a:ty, { $i:item }) => {
        impl cxx_qt::Constructor<$a> for $t {
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

pub trait Convert<T> {
    fn convert(&self) -> T;
}

impl Convert<RgbColor> for QColor {
    fn convert(&self) -> RgbColor {
        let rgb = self.to_rgb();
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
        QColor::from_rgb(i32::from(self.r), i32::from(self.g), i32::from(self.b))
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

impl Convert<Option<String>> for QString {
    fn convert(&self) -> Option<String> {
        if self.is_empty() {
            None
        } else {
            Some(String::from(self))
        }
    }
}

impl Convert<QString> for Option<String> {
    fn convert(&self) -> QString {
        match self {
            Some(s) => QString::from(s),
            None => QString::default(),
        }
    }
}

impl Convert<QList<QString>> for [String] {
    fn convert(&self) -> QList<QString> {
        let mut list = QList::default();
        list.reserve(self.len() as isize);
        for item in self {
            list.append(QString::from(item));
        }
        list
    }
}

impl Convert<QStringList> for [String] {
    fn convert(&self) -> QStringList {
        let list: QList<QString> = self.convert();
        QStringList::from(&list)
    }
}

impl Convert<Vec<String>> for QList<QString> {
    fn convert(&self) -> Vec<String> {
        self.iter().map(String::from).collect()
    }
}

impl Convert<Vec<String>> for QStringList {
    fn convert(&self) -> Vec<String> {
        QList::from(self).convert()
    }
}

impl Convert<QList<QString>> for [&str] {
    fn convert(&self) -> QList<QString> {
        let mut list = QList::default();
        list.reserve(self.len() as isize);
        for item in self {
            list.append(QString::from(*item));
        }
        list
    }
}

impl Convert<QStringList> for [&str] {
    fn convert(&self) -> QStringList {
        let list: QList<QString> = self.convert();
        QStringList::from(&list)
    }
}

impl<const N: usize> Convert<[String; N]> for QList<QString> {
    /// # Panics
    ///
    /// Panics if `self.len()` is not `N`.
    fn convert(&self) -> [String; N] {
        let vec: Vec<String> = self.convert();
        match vec.try_into() {
            Ok(array) => array,
            Err(e) => panic!("expected length {N}, got length {}", e.len()),
        }
    }
}

impl<const N: usize> Convert<[String; N]> for QStringList {
    fn convert(&self) -> [String; N] {
        QList::from(self).convert()
    }
}

impl Convert<QByteArray> for PathBuf {
    fn convert(&self) -> QByteArray {
        QByteArray::from(self.as_os_str().as_encoded_bytes())
    }
}

impl Convert<PathBuf> for QByteArray {
    fn convert(&self) -> PathBuf {
        // SAFETY: Input values are produced by the above use of `as_encoded_bytes`.
        unsafe { OsStr::from_encoded_bytes_unchecked(self.as_slice()) }.into()
    }
}

impl<T, U> Convert<Vec<T>> for QVector<U>
where
    U: cxx_qt_lib::QVectorElement + Convert<T>,
{
    fn convert(&self) -> Vec<T> {
        self.iter().map(Convert::convert).collect()
    }
}

impl<T, U, const N: usize> Convert<[T; N]> for QVector<U>
where
    U: cxx_qt_lib::QVectorElement + Convert<T>,
{
    /// # Panics
    ///
    /// Panics if `self.len()` is not `N`.
    fn convert(&self) -> [T; N] {
        let vec: Vec<T> = self.convert();
        match vec.try_into() {
            Ok(array) => array,
            Err(e) => panic!("expected length {N}, got length {}", e.len()),
        }
    }
}

impl<T, U> Convert<QVector<T>> for [U]
where
    T: ExternType<Kind = Trivial> + cxx_qt_lib::QVectorElement,
    U: Convert<T>,
{
    fn convert(&self) -> QVector<T> {
        let mut qvec = QVector::default();
        qvec.reserve(self.len() as isize);
        for item in self {
            qvec.append(item.convert());
        }
        qvec
    }
}
