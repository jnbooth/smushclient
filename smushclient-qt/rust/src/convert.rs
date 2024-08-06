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
