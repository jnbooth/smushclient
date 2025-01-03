macro_rules! adapter {
    ($rust:ident, $ffi:ty) => {
        #[repr(transparent)]
        pub struct $rust<'a> {
            inner: std::pin::Pin<&'a mut $ffi>,
        }

        impl<'a> From<std::pin::Pin<&'a mut $ffi>> for $rust<'a> {
            fn from(value: std::pin::Pin<&'a mut $ffi>) -> Self {
                Self { inner: value }
            }
        }

        impl<'a> From<$rust<'a>> for std::pin::Pin<&'a mut $ffi> {
            fn from(value: $rust<'a>) -> Self {
                value.inner
            }
        }

        impl<'a> $rust<'a> {
            fn as_mut(&mut self) -> std::pin::Pin<&mut $ffi> {
                self.inner.as_mut()
            }
        }
    };
}

mod document;
pub use document::{DocumentAdapter, QColorPair};

mod socket;
pub use socket::SocketAdapter;

mod timekeeper;
pub use timekeeper::TimekeeperAdapter;
