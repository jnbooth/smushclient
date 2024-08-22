use std::ffi::c_char;
use std::fs::File;
use std::io::BufRead;
use std::pin::Pin;

use crate::ffi;
use crate::handler::ClientHandler;
use crate::sync::NonBlockingMutex;
use crate::world::WorldRust;
use cxx_qt_lib::QString;
use mud_transformer::Transformer;
use smushclient::world::PersistError;
use smushclient::{SmushClient, World};

#[derive(Default)]
pub struct SmushClientRust {
    done: bool,
    client: SmushClient,
    transformer: Transformer,
    buf: Vec<u8>,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
}

impl SmushClientRust {
    pub fn load_world(&mut self, path: &QString, world: &mut WorldRust) -> bool {
        let Ok(worldfile) = Self::try_load_world(path) else {
            return false;
        };
        world.populate(&worldfile);
        let config = self.client.set_world(worldfile);
        self.transformer.set_config(config);
        true
    }

    fn try_load_world(path: &QString) -> Result<World, PersistError> {
        let file = File::open(String::from(path))?;
        World::load(file)
    }

    pub fn populate_world(&self, world: &mut WorldRust) {
        world.populate(self.client.world());
    }

    pub fn set_world(&mut self, world: &WorldRust) {
        let config = self.client.set_world(world.into());
        self.transformer.set_config(config);
    }

    pub fn read(
        &mut self,
        mut device: Pin<&mut ffi::QIODevice>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        if self.done {
            return -1;
        }
        let mut handler = ClientHandler { doc };

        let output_lock = self.output_lock.lock();
        let buf_ptr = self.buf.as_mut_ptr().cast::<c_char>();
        let buf_len = i64::try_from(self.buf.len()).unwrap();
        let mut total_read = 0;
        loop {
            // SAFETY: Device will not read past buf_len.
            let n = unsafe { device.as_mut().read(buf_ptr, buf_len) };
            if n == 0 {
                break;
            }
            if n == -1 {
                self.done = true;
                self.client
                    .receive(self.transformer.flush_output(), &mut handler);
                return total_read;
            }
            total_read += n;
            let i = usize::try_from(n).unwrap();
            let (received, buf) = self.buf.split_at_mut(i);
            self.transformer.receive(received, buf).unwrap();
        }
        self.client
            .receive(self.transformer.drain_output(), &mut handler);
        drop(output_lock);

        let input_lock = self.input_lock.lock();
        if let Some(mut drain) = self.transformer.drain_input() {
            loop {
                let drain_buf = drain.fill_buf().unwrap();
                if drain_buf.is_empty() {
                    break;
                }
                let drain_ptr = drain_buf.as_ptr().cast::<c_char>();
                let drain_len = i64::try_from(drain_buf.len()).unwrap();
                // SAFETY: Device will not write past drain_len.
                let n = unsafe { device.as_mut().write(drain_ptr, drain_len) };
                let i = usize::try_from(n).unwrap();
                drain.consume(i);
            }
        }
        drop(input_lock);

        total_read
    }
}

impl ffi::SmushClient {
    pub fn load_world(self: Pin<&mut Self>, path: &QString, world: Pin<&mut ffi::World>) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .load_world(path, &mut world.cxx_qt_ffi_rust_mut())
    }

    pub fn populate_world(&self, world: Pin<&mut ffi::World>) {
        self.cxx_qt_ffi_rust()
            .populate_world(&mut world.cxx_qt_ffi_rust_mut());
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) {
        self.cxx_qt_ffi_rust_mut()
            .set_world(world.cxx_qt_ffi_rust());
    }

    pub fn read(
        self: Pin<&mut Self>,
        device: Pin<&mut ffi::QIODevice>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device, doc)
    }
}
