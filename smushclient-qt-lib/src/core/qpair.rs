use std::hash::Hash;

use cxx::{ExternType, type_id};
use cxx_qt_lib::QColor;

/// Typedef for `std::pair<T1, T1>`.
#[repr(C)]
#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct QPair<T1, T2> {
    pub first: T1,
    pub second: T2,
}

impl<T1, T2> From<(T1, T2)> for QPair<T1, T2> {
    fn from((first, second): (T1, T2)) -> Self {
        Self { first, second }
    }
}

impl<T1, T2> From<QPair<T1, T2>> for (T1, T2) {
    fn from(value: QPair<T1, T2>) -> Self {
        (value.first, value.second)
    }
}

impl<'a, T1, T2> From<&'a QPair<T1, T2>> for (&'a T1, &'a T2) {
    fn from(value: &'a QPair<T1, T2>) -> Self {
        (&value.first, &value.second)
    }
}

macro_rules! impl_extern {
    ( $firstTypeName:ty, $secondTypeName:ty, $typeId:literal ) => {
        // SAFETY: Static checks on the C++ side to ensure the size is the same.
        unsafe impl ExternType for QPair<$firstTypeName, $secondTypeName> {
            type Id = type_id!($typeId);
            type Kind = cxx::kind::Trivial;
        }
    };
}

impl_extern!(QColor, QColor, "QPair_QColor_QColor");
