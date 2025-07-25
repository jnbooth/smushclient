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
pub(crate) use impl_convert;

macro_rules! impl_convert_self {
    ($($t:ty),+) => {
        $(impl_convert!($t, $t);)+
    }
}
pub(crate) use impl_convert_self;

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

macro_rules! impl_convert_enum_opt {
    ($ffi:ty, $rust:ty, $($variant:ident),+ $(,)?) => {
        impl From<Option<$rust>> for $ffi {
            fn from(value: Option<$rust>) -> Self {
                match value {
                    None => Self::None,
                    $(Some(<$rust>::$variant) => Self::$variant),+
                }
            }
        }
        impl From<$ffi> for Option<$rust> {
            fn from(value: $ffi) -> Self {
                match value {
                    <$ffi>::None => None,
                    $(<$ffi>::$variant => Some(<$rust>::$variant)),+,
                    #[allow(unreachable_patterns)]
                    _ => unreachable!()
                }
            }
        }
        impl_convert!($ffi, Option<$rust>);
    }
}
pub(crate) use impl_convert_enum_opt;

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

impl Convert<Option<String>> for String {
    fn from_ffi(value: Self) -> Option<String> {
        if value.is_empty() {
            None
        } else {
            Some(value)
        }
    }

    fn to_ffi(value: Option<String>) -> Self {
        value.unwrap_or_default()
    }
}

impl<T, Ffi: Convert<T>> Convert<Option<T>> for Option<Ffi> {
    fn from_ffi(value: Self) -> Option<T> {
        value.map(Convert::from_ffi)
    }

    fn to_ffi(value: Option<T>) -> Self {
        value.map(Convert::to_ffi)
    }
}

impl<T, Ffi: Convert<T>> Convert<Vec<T>> for Vec<Ffi> {
    fn from_ffi(value: Self) -> Vec<T> {
        value.into_iter().map(Convert::from_ffi).collect()
    }

    fn to_ffi(value: Vec<T>) -> Self {
        value.into_iter().map(Convert::to_ffi).collect()
    }
}

impl<T, Ffi: Convert<T>, const N: usize> Convert<[T; N]> for Vec<Ffi> {
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

impl_convert_self!(bool, f32, f64, i16, i32, String, u16, u32, u8, usize);
