#[macro_export]
macro_rules! unsafe_impl_qflag {
    ( $typeName:ty, $typeId:literal ) => {
        unsafe_impl_qflag!($typeName, $typeId, i32);
    };
    ( $typeName:ty, $typeId:literal, $repr:ident ) => {
        // SAFETY: Static checks on the C++ side.
        unsafe impl ::cxx_qt_lib::QFlag for $typeName {
            type TypeId = ::cxx::type_id!($typeId);
            type Repr = $repr;

            fn to_repr(self) -> Self::Repr {
                self.repr
            }
        }

        impl ::std::ops::BitOr for $typeName {
            type Output = ::cxx_qt_lib::QFlags<$typeName>;

            fn bitor(self, other: Self) -> Self::Output {
                ::cxx_qt_lib::QFlags::from(self) | other
            }
        }

        impl ::std::ops::BitOr<::cxx_qt_lib::QFlags<$typeName>> for $typeName {
            type Output = ::cxx_qt_lib::QFlags<$typeName>;

            fn bitor(self, other: ::cxx_qt_lib::QFlags<$typeName>) -> Self::Output {
                other | self
            }
        }
    };
}
