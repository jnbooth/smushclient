use std::fs::File;
use std::io::Read;
use std::pin::Pin;

use crate::convert::Convert;
use crate::ffi;
use crate::handler::ClientHandler;
use crate::sync::NonBlockingMutex;
use crate::world::WorldRust;
use cxx_qt::Initialize;
use cxx_qt_lib::{QColor, QString};
use enumeration::EnumSet;
use mud_transformer::{Tag, Transformer};
use smushclient::world::PersistError;
use smushclient::{SmushClient, World};

const BUF_LEN: usize = 1024 * 20;
const BUF_MIDPOINT: usize = BUF_LEN / 2;

const SUPPORTED_TAGS: EnumSet<Tag> = enums![
    Tag::Bold,
    Tag::Color,
    Tag::Font,
    Tag::H1,
    Tag::H2,
    Tag::H3,
    Tag::H4,
    Tag::H5,
    Tag::H6,
    Tag::Highlight,
    Tag::Hr,
    Tag::Hyperlink,
    Tag::Italic,
    Tag::Send,
    Tag::Strikeout,
    Tag::Underline
];

#[derive(Default)]
pub struct SmushClientRust {
    done: bool,
    client: SmushClient,
    transformer: Transformer,
    buf: Vec<u8>,
    custom_color: QColor,
    error_color: QColor,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
}

impl SmushClientRust {
    fn initialize(&mut self) {
        self.buf.resize(BUF_LEN, 0);
        self.client.set_supported_tags(SUPPORTED_TAGS);
    }

    pub fn load_world(&mut self, path: &QString, world: &mut WorldRust) -> bool {
        let Ok(worldfile) = Self::try_load_world(path) else {
            return false;
        };
        *world = WorldRust::from(&worldfile);
        self.apply_world(worldfile);
        true
    }

    fn try_load_world(path: &QString) -> Result<World, PersistError> {
        let file = File::open(String::from(path))?;
        World::load(file)
    }

    pub fn save_world(&self, path: &QString) -> bool {
        self.try_save_world(path).is_ok()
    }

    fn try_save_world(&self, path: &QString) -> Result<(), PersistError> {
        let file = File::create(String::from(path))?;
        self.client.world().save(file)
    }

    pub fn populate_world(&self, world: &mut WorldRust) {
        *world = WorldRust::from(self.client.world());
    }

    pub fn set_world(&mut self, world: &WorldRust) {
        let world = World::from(world);
        self.apply_world(world);
    }

    fn apply_world(&mut self, world: World) {
        self.custom_color = world.custom_color.convert();
        self.error_color = world.error_color.convert();
        let config = self.client.set_world(world);
        self.transformer.set_config(config);
    }

    pub fn read(&mut self, device: Pin<&mut ffi::QTcpSocket>, doc: Pin<&mut ffi::Document>) -> i64 {
        if self.done {
            return -1;
        }
        let mut handler = ClientHandler {
            doc: doc.into(),
            socket: device.into(),
            custom_color: &self.custom_color,
            error_color: &self.error_color,
        };

        let output_lock = self.output_lock.lock();
        let mut total_read = 0;
        loop {
            let n = match handler.socket.read(&mut self.buf[..BUF_MIDPOINT]) {
                Ok(0) => break,
                Ok(n) => n,
                Err(e) => {
                    self.done = true;
                    handler.display_error(&e.to_string());
                    self.client
                        .receive(self.transformer.flush_output(), &mut handler);
                    return i64::try_from(total_read).unwrap();
                }
            };
            total_read += n;
            let (received, buf) = self.buf.split_at_mut(n);
            if let Err(e) = self.transformer.receive(received, buf) {
                handler.display_error(&e.to_string());
                return -1;
            }
        }
        self.client
            .receive(self.transformer.drain_output(), &mut handler);
        handler.doc.scroll_to_bottom();
        drop(output_lock);

        let input_lock = self.input_lock.lock();
        if let Some(mut drain) = self.transformer.drain_input() {
            if let Err(e) = drain.write_all_to(&mut handler.socket) {
                handler.display_error(&e.to_string());
                return -1;
            }
        }
        drop(input_lock);

        i64::try_from(total_read).unwrap()
    }
}

impl Initialize for ffi::SmushClient {
    fn initialize(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut().initialize();
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

    pub fn save_world(&self, path: &QString) -> bool {
        self.cxx_qt_ffi_rust().save_world(path)
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) {
        self.cxx_qt_ffi_rust_mut()
            .set_world(world.cxx_qt_ffi_rust());
    }

    pub fn read(
        self: Pin<&mut Self>,
        device: Pin<&mut ffi::QTcpSocket>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device, doc)
    }
}
