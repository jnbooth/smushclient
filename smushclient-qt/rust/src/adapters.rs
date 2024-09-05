#![allow(clippy::cast_possible_truncation)]
#![allow(clippy::cast_possible_wrap)]
use cxx_qt_lib::{QColor, QString};

use crate::ffi;
use std::ffi::c_char;
use std::io::{self, Read, Write};
use std::pin::Pin;

macro_rules! impl_adapter {
    ($rust:ident, $ffi:ty) => {
        #[repr(transparent)]
        pub struct $rust<'a> {
            inner: Pin<&'a mut $ffi>,
        }

        impl<'a> From<Pin<&'a mut $ffi>> for $rust<'a> {
            fn from(value: Pin<&'a mut $ffi>) -> Self {
                Self { inner: value }
            }
        }

        impl<'a> From<$rust<'a>> for Pin<&'a mut $ffi> {
            fn from(value: $rust<'a>) -> Self {
                value.inner
            }
        }

        impl<'a> $rust<'a> {
            fn as_mut(&mut self) -> Pin<&mut $ffi> {
                self.inner.as_mut()
            }
        }
    };
}

impl_adapter!(SocketAdapter, ffi::QTcpSocket);

impl<'a> Read for SocketAdapter<'a> {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let buf_ptr = buf.as_mut_ptr().cast::<c_char>();
        let buf_len = buf.len() as i64;
        // SAFETY: Device will not read past buf.len().
        let n = unsafe { self.as_mut().read(buf_ptr, buf_len) };
        if n == -1 {
            Err(io::ErrorKind::BrokenPipe.into())
        } else {
            Ok(n as usize)
        }
    }
}

impl<'a> Write for SocketAdapter<'a> {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        let bytes_ptr = buf.as_ptr().cast::<c_char>();
        let bytes_len = buf.len() as i64;
        // SAFETY: Device will not write past bytes.len().
        let n = unsafe { self.as_mut().write(bytes_ptr, bytes_len) };
        if n == -1 {
            Err(io::ErrorKind::BrokenPipe.into())
        } else {
            Ok(n as usize)
        }
    }

    fn flush(&mut self) -> io::Result<()> {
        unsafe { self.as_mut().flush() };
        Ok(())
    }
}

impl_adapter!(DocumentAdapter, ffi::Document);

impl<'a> DocumentAdapter<'a> {
    pub fn scroll_to_bottom(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().scroll_to_bottom() }
    }

    pub fn append_line(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_line() }
    }

    pub fn append_plaintext(&mut self, text: &QString, palette: i32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_plaintext(text, palette) };
    }

    pub fn append_text(
        &mut self,
        text: &QString,
        style: u16,
        foreground: &QColor,
        background: &QColor,
    ) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.as_mut()
                .append_text(text, style, foreground, background);
        }
    }

    pub fn append_link(
        &mut self,
        text: &QString,
        style: u16,
        foreground: &QColor,
        background: &QColor,
        link: &ffi::Link,
    ) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.as_mut()
                .append_link(text, style, foreground, background, link);
        }
    }

    pub fn display_status_message(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().display_status_message(text) };
    }

    pub fn set_input(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().set_input(text) };
    }
}

impl_adapter!(TreeBuilderAdapter, ffi::TreeBuilder);

pub trait ColumnInsertable {
    unsafe fn insert_column(self, builder: Pin<&mut ffi::TreeBuilder>);
}

impl ColumnInsertable for &QString {
    unsafe fn insert_column(self, builder: Pin<&mut ffi::TreeBuilder>) {
        builder.add_column(self);
    }
}

impl ColumnInsertable for i16 {
    unsafe fn insert_column(self, builder: Pin<&mut ffi::TreeBuilder>) {
        builder.add_column_i16(self);
    }
}

impl<'a> TreeBuilderAdapter<'a> {
    pub fn start_group(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_group(text) };
    }

    pub fn start_item(&mut self, value: usize) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_item(value) };
    }

    pub fn add_column<T: ColumnInsertable>(&mut self, value: T) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { value.insert_column(self.as_mut()) };
    }
}
