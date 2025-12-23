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

#[macro_export]
macro_rules! wrap_qsignal {
    (
        $(#[$attr:meta])*
        $signal:ident ( $qsignal:ident ) ( $($i:ident : $t:ty),* );
        $connect:ident ( $qconnect:ident );
        $on:ident ( $qon:ident );
        $name:literal
    ) => {
        $(#[$attr])*
        pub fn $signal(self: Pin<&mut Self>, $($i : $t),*) {
           self.$qsignal($($i.into()),*);
        }

        #[doc = "Connect the given function pointer to the signal "]
        #[doc = $name]
        #[doc = " so that when the signal is emitted the function pointer is executed."]
        pub fn $connect<F: FnMut(Pin<&mut Self>, $($t),*) + 'static + Send>(
            self: Pin<&mut Self>,
            mut closure: F,
            conn_type: cxx_qt::ConnectionType,
        ) -> cxx_qt::QMetaObjectConnectionGuard {
            self.$qconnect(move |this, $($i),*| closure(this, $($i.into()),*), conn_type)
        }


        #[doc = "Connect the given function pointer to the signal "]
        #[doc = $name]
        #[doc = ", so that when the signal is emitted the function pointer is executed."]
        #[doc = ""]
        #[doc = "Note that this method uses a AutoConnection connection type."]
        pub fn $on<F: FnMut(Pin<&mut Self>, $($t),*) + 'static + Send>(
            self: Pin<&mut Self>,
            mut closure: F,
        ) -> cxx_qt::QMetaObjectConnectionGuard {
            self.$qon(move |this, $($i),*| closure(this, $($i.into()),*))
        }
    };
}
