use std::io::BufRead;
use std::os::raw::c_char;
use std::pin::Pin;

use crate::output::RustOutputFragment;

use crate::ffi;
use crate::handler::ClientHandler;
use crate::sync::NonBlockingMutex;
use mud_transformer::Transformer;
use smushclient::SmushClient;

#[derive(Default)]
pub struct SmushClientRust {
    done: bool,
    client: SmushClient,
    transformer: Transformer,
    buf: Vec<u8>,
    output: ClientHandler,
    cursor: usize,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
}

impl SmushClientRust {
    pub fn read(&mut self, mut device: Pin<&mut ffi::QIODevice>) -> i64 {
        if self.done {
            return -1;
        }
        if self.cursor >= self.output.len() {
            self.cursor = 0;
            self.output.clear();
        }

        let output_lock = self.output_lock.lock();
        let buf_ptr = self.buf.as_mut_ptr().cast::<c_char>();
        let buf_len = self.buf.len() as i64;
        let mut total_read = 0;
        loop {
            // SAFETY: Device will not read past buf_len.
            let n = unsafe { device.as_mut().read(buf_ptr, buf_len) };
            if n == 0 {
                break;
            }
            total_read += n;
            if n == -1 {
                self.done = true;
                self.client
                    .receive(self.transformer.flush_output(), &mut self.output);
                return total_read;
            }
            let (received, buf) = self.buf.split_at_mut(n as usize);
            self.transformer.receive(received, buf).unwrap();
        }
        self.client
            .receive(self.transformer.drain_output(), &mut self.output);
        drop(output_lock);

        let input_lock = self.input_lock.lock();
        if let Some(mut drain) = self.transformer.drain_input() {
            loop {
                let drain_buf = drain.fill_buf().unwrap();
                if drain_buf.is_empty() {
                    break;
                }
                let drain_ptr = drain_buf.as_ptr().cast::<c_char>();
                let drain_len = drain_buf.len() as i64;
                // SAFETY: Device will not write past drain_len.
                let n = unsafe { device.as_mut().write(drain_ptr, drain_len) };
                drain.consume(n as usize);
            }
        }
        drop(input_lock);

        total_read
    }

    pub fn try_next(&mut self) -> bool {
        if self.cursor < self.output.len() {
            self.cursor += 1;
            true
        } else {
            false
        }
    }

    pub fn next(&self) -> &RustOutputFragment {
        (&self.output[self.cursor - 1]).into()
    }
}

impl ffi::SmushClient {
    pub fn read(self: Pin<&mut Self>, device: Pin<&mut ffi::QIODevice>) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device)
    }

    pub fn try_next(self: Pin<&mut Self>) -> bool {
        self.cxx_qt_ffi_rust_mut().try_next()
    }

    pub fn next(&self) -> &RustOutputFragment {
        self.cxx_qt_ffi_rust().next()
    }
}
