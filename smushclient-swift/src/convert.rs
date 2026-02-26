pub trait Convert<T> {
    fn from_ffi(value: Self) -> T;
    fn to_ffi(value: T) -> Self;
}

macro_rules! impl_convert {
    ($ffi:ty, $rust:ty) => {
        impl Convert<$rust> for $ffi {
            fn from_ffi(value: Self) -> $rust {
                <$rust>::from(value)
            }

            fn to_ffi(value: $rust) -> Self {
                Self::from(value)
            }
        }
    };
}

use std::borrow::Cow;

pub(crate) use impl_convert;

macro_rules! impl_convert_self {
    ($($t:ty),+) => {
        $(impl_convert!($t, $t);)+
    }
}

macro_rules! impl_convert_enum {
    ($ffi:ty, $rust:ty, $($variant:ident),+ $(,)?) => {
        impl From<$rust> for $ffi {
            fn from(value: $rust) -> Self {
                match value {
                    $(<$rust>::$variant => Self::$variant),+
                }
            }
        }
        impl From<$ffi> for $rust {
            fn from(value: $ffi) -> Self {
                match value {
                    $(<$ffi>::$variant => Self::$variant),+,
                    #[allow(unreachable_patterns)]
                    _ => unreachable!()
                }
            }
        }
        impl_convert!($ffi, $rust);
    }
}
pub(crate) use impl_convert_enum;

macro_rules! impl_convert_struct {
    ($ffi:ty, $rust:ty, $($prop:ident),+ $(,)?) => {
        impl From<$rust> for $ffi {
            fn from(value: $rust) -> Self {
                Self {
                    $($prop: Convert::to_ffi(value.$prop)),*
                }
            }
        }
        impl From<$ffi> for $rust {
            fn from(value: $ffi) -> Self {
                Self {
                    $($prop: Convert::from_ffi(value.$prop)),*
                }
            }
        }
        impl_convert!($ffi, $rust);
    }
}
pub(crate) use impl_convert_struct;

impl<T, Ffi> Convert<Option<T>> for Option<Ffi>
where
    Ffi: Convert<T>,
{
    fn from_ffi(value: Self) -> Option<T> {
        value.map(Convert::from_ffi)
    }

    fn to_ffi(value: Option<T>) -> Self {
        value.map(Convert::to_ffi)
    }
}

impl<T, Ffi> Convert<Cow<'static, T>> for Ffi
where
    T: ToOwned + ?Sized,
    Ffi: Convert<T::Owned>,
{
    fn from_ffi(value: Self) -> Cow<'static, T> {
        Cow::Owned(Convert::from_ffi(value))
    }

    fn to_ffi(value: Cow<'static, T>) -> Self {
        Convert::to_ffi(value.into_owned())
    }
}

impl<T, Ffi> Convert<Vec<T>> for Vec<Ffi>
where
    Ffi: Convert<T>,
{
    fn from_ffi(value: Self) -> Vec<T> {
        value.into_iter().map(Convert::from_ffi).collect()
    }

    fn to_ffi(value: Vec<T>) -> Self {
        value.into_iter().map(Convert::to_ffi).collect()
    }
}

impl<T, Ffi, const N: usize> Convert<[T; N]> for Vec<Ffi>
where
    Ffi: Convert<T>,
{
    /// # Panics
    ///
    /// Panics if `self.len()` is not `N`.
    fn from_ffi(value: Self) -> [T; N] {
        let vec: Vec<T> = value.into_iter().map(Convert::from_ffi).collect();
        match vec.try_into() {
            Ok(array) => array,
            Err(e) => panic!("expected length {N}, got length {}", e.len()),
        }
    }

    fn to_ffi(value: [T; N]) -> Self {
        value.into_iter().map(Convert::to_ffi).collect()
    }
}

impl_convert_self!(bool, f32, f64, i16, i32, i64, String, u16, u32, u8, usize);
