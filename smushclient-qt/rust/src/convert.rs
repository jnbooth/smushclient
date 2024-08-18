macro_rules! impl_convert_enum {
    ($ffi:ty, $rust:path, $($variant:ident),+ $(,)?) => {
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
    }
}
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
    }
}

macro_rules! binding {
    ($f:ident, $t:ty) => {
        #[derive(Debug)]
        #[repr(transparent)]
        pub struct $f {
            inner: $t,
        }

        impl From<&$t> for &$f {
            fn from(inner: &$t) -> Self {
                // SAFETY: #[repr(transparent)]
                unsafe { &*(inner as *const $t).cast::<Self>() }
            }
        }
    };
}
