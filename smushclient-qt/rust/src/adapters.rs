#![allow(clippy::cast_possible_truncation)]
#![allow(clippy::cast_possible_wrap)]
use cxx_qt_lib::{QColor, QString};

use crate::ffi;
use std::ffi::c_char;
use std::io::{self, Read, Write};
use std::pin::Pin;

#[repr(transparent)]
pub struct SocketAdapter<'a> {
    inner: Pin<&'a mut ffi::QTcpSocket>,
}

impl<'a> From<Pin<&'a mut ffi::QTcpSocket>> for SocketAdapter<'a> {
    fn from(value: Pin<&'a mut ffi::QTcpSocket>) -> Self {
        Self { inner: value }
    }
}

impl<'a> From<SocketAdapter<'a>> for Pin<&'a mut ffi::QTcpSocket> {
    fn from(value: SocketAdapter<'a>) -> Self {
        value.inner
    }
}

impl<'a> Read for SocketAdapter<'a> {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let buf_ptr = buf.as_mut_ptr().cast::<c_char>();
        let buf_len = buf.len() as i64;
        // SAFETY: Device will not read past buf.len().
        let n = unsafe { self.inner.as_mut().read(buf_ptr, buf_len) };
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
        let n = unsafe { self.inner.as_mut().write(bytes_ptr, bytes_len) };
        if n == -1 {
            Err(io::ErrorKind::BrokenPipe.into())
        } else {
            Ok(n as usize)
        }
    }

    fn flush(&mut self) -> io::Result<()> {
        unsafe { self.inner.as_mut().flush() };
        Ok(())
    }
}

#[repr(transparent)]
pub struct DocumentAdapter<'a> {
    inner: Pin<&'a mut ffi::Document>,
}

impl<'a> From<Pin<&'a mut ffi::Document>> for DocumentAdapter<'a> {
    fn from(value: Pin<&'a mut ffi::Document>) -> Self {
        Self { inner: value }
    }
}

impl<'a> From<DocumentAdapter<'a>> for Pin<&'a mut ffi::Document> {
    fn from(value: DocumentAdapter<'a>) -> Self {
        value.inner
    }
}

impl<'a> DocumentAdapter<'a> {
    pub fn scroll_to_bottom(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().scroll_to_bottom() }
    }

    pub fn append_line(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().append_line() }
    }

    pub fn append_plaintext(&mut self, text: &QString, color: &QColor) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().append_plaintext(text, color) };
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
            self.inner
                .as_mut()
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
            self.inner
                .as_mut()
                .append_link(text, style, foreground, background, link);
        }
    }

    pub fn set_input(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().set_input(text) };
    }
}
