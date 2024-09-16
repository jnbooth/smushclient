#![allow(clippy::cast_sign_loss)]
use crate::ffi;
use std::ffi::c_char;
use std::io::{self, Read, Write};

adapter!(SocketAdapter, ffi::QTcpSocket);

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
