use std::collections::HashMap;
use std::ffi::c_char;
use std::fs::File;
use std::io::{self, Read};
use std::pin::Pin;
use std::{ptr, slice};

use crate::convert::Convert;
use crate::ffi;
use crate::get_info::InfoVisitorQVariant;
use crate::handler::ClientHandler;
use crate::sync::NonBlockingMutex;
use crate::world::WorldRust;
use cxx_qt_lib::{QColor, QList, QString, QStringList, QVariant, QVector};
use enumeration::EnumSet;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{Tag, Transformer};
use smushclient::world::PersistError;
use smushclient::{SmushClient, World};
use smushclient_plugins::{Alias, PluginIndex, Timer, Trigger};

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

pub struct SmushClientRust {
    done: bool,
    client: SmushClient,
    transformer: Transformer,
    buf: Vec<u8>,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
    send: Vec<QString>,
    palette: HashMap<RgbColor, i32>,
}

impl Default for SmushClientRust {
    fn default() -> Self {
        Self {
            done: false,
            client: SmushClient::new(World::default(), SUPPORTED_TAGS),
            transformer: Transformer::default(),
            buf: vec![0; BUF_LEN],
            input_lock: NonBlockingMutex::default(),
            output_lock: NonBlockingMutex::default(),
            send: Vec::new(),
            palette: HashMap::with_capacity(166),
        }
    }
}

impl SmushClientRust {
    pub fn load_world(&mut self, path: &QString) -> Result<WorldRust, PersistError> {
        let file = File::open(String::from(path).as_str())?;
        let worldfile = World::load(file)?;
        let world = WorldRust::from(&worldfile);
        self.apply_world(worldfile);
        Ok(world)
    }

    pub fn save_world(&self, path: &QString) -> Result<(), PersistError> {
        let file = File::create(String::from(path).as_str())?;
        self.client.world().save(file)
    }

    pub fn load_variables(&mut self, path: &QString) -> Result<bool, PersistError> {
        let file = match File::open(String::from(path).as_str()) {
            Err(err) if err.kind() == io::ErrorKind::NotFound => return Ok(false),
            file => file?,
        };
        self.client.load_variables(file)?;
        Ok(true)
    }

    pub fn save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        if !self.client.has_variables() {
            return Ok(false);
        }
        let file = File::create(String::from(path).as_str())?;
        self.client.save_variables(file)?;
        Ok(true)
    }

    pub fn populate_world(&self, world: &mut WorldRust) {
        *world = WorldRust::from(self.client.world());
    }

    pub fn set_world(&mut self, world: &WorldRust) -> bool {
        let Ok(world) = world.try_into() else {
            return false;
        };
        self.apply_world(world);
        true
    }

    pub fn palette(&self) -> Vec<QColor> {
        self.client
            .world()
            .palette()
            .iter()
            .map(Convert::convert)
            .collect()
    }

    pub fn plugin_info(&self, index: PluginIndex, info_type: u8) -> QVariant {
        self.client
            .plugin_info::<InfoVisitorQVariant>(index, info_type)
    }

    pub fn plugin_scripts(&self) -> QStringList {
        let mut list = QList::default();
        for plugin in self.client.plugins() {
            list.append(QString::from(&plugin.metadata.id));
            list.append(QString::from(&plugin.metadata.name));
            list.append(QString::from(&plugin.script));
        }
        QStringList::from(&list)
    }

    fn apply_world(&mut self, world: World) {
        self.palette.clear();
        for (i, color) in world.palette().iter().enumerate() {
            self.palette.insert(*color, i32::try_from(i).unwrap());
        }
        let config = self.client.set_world(world);
        self.transformer.set_config(config);
    }

    pub fn read(&mut self, device: Pin<&mut ffi::QTcpSocket>, doc: Pin<&mut ffi::Document>) -> i64 {
        if self.done {
            return -1;
        }
        self.send.clear();
        let mut handler = ClientHandler {
            doc: doc.into(),
            socket: device.into(),
            palette: &self.palette,
            send: &mut self.send,
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
        handler.output_sends();
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

impl ffi::SmushClient {
    pub fn load_world(
        self: Pin<&mut Self>,
        path: &QString,
        world: Pin<&mut ffi::World>,
    ) -> Result<(), PersistError> {
        *world.cxx_qt_ffi_rust_mut() = self.cxx_qt_ffi_rust_mut().load_world(path)?;
        Ok(())
    }

    pub fn save_world(&self, path: &QString) -> Result<(), PersistError> {
        self.cxx_qt_ffi_rust().save_world(path)
    }

    pub fn load_variables(self: Pin<&mut Self>, path: &QString) -> Result<bool, PersistError> {
        self.cxx_qt_ffi_rust_mut().load_variables(path)
    }

    pub fn save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.cxx_qt_ffi_rust().save_variables(path)
    }

    pub fn populate_world(&self, world: Pin<&mut ffi::World>) {
        self.cxx_qt_ffi_rust()
            .populate_world(&mut world.cxx_qt_ffi_rust_mut());
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) -> bool {
        return self
            .cxx_qt_ffi_rust_mut()
            .set_world(world.cxx_qt_ffi_rust());
    }

    pub fn palette(&self) -> QVector<QColor> {
        QVector::from(&self.cxx_qt_ffi_rust().palette())
    }

    pub fn plugin_info(&self, index: PluginIndex, info_type: u8) -> QVariant {
        self.cxx_qt_ffi_rust().plugin_info(index, info_type)
    }

    pub fn plugin_scripts(&self) -> QStringList {
        self.cxx_qt_ffi_rust().plugin_scripts()
    }

    pub fn read(
        self: Pin<&mut Self>,
        device: Pin<&mut ffi::QTcpSocket>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device, doc)
    }

    pub fn is_alias(&self, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .sender_exists::<Alias>(&label.to_string())
    }

    pub fn is_timer(&self, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .sender_exists::<Timer>(&label.to_string())
    }

    pub fn is_trigger(&self, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .sender_exists::<Trigger>(&label.to_string())
    }

    pub fn set_alias_enabled(self: Pin<&mut Self>, label: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Alias>(&label.to_string(), enabled)
    }

    pub fn set_aliases_enabled(self: Pin<&mut Self>, group: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Alias>(&group.to_string(), enabled)
    }

    pub fn set_plugin_enabled(self: Pin<&mut Self>, index: PluginIndex, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_plugin_enabled(index, enabled)
    }

    pub fn set_timer_enabled(self: Pin<&mut Self>, label: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Timer>(&label.to_string(), enabled)
    }

    pub fn set_timers_enabled(self: Pin<&mut Self>, group: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Timer>(&group.to_string(), enabled)
    }

    pub fn set_trigger_enabled(self: Pin<&mut Self>, label: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Trigger>(&label.to_string(), enabled)
    }

    pub fn set_triggers_enabled(self: Pin<&mut Self>, group: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Trigger>(&group.to_string(), enabled)
    }

    /// # Safety
    ///
    /// Refer to the safety documentation for [`std::slice::from_raw_parts`].
    pub unsafe fn get_variable(
        &self,
        index: PluginIndex,
        key: *const c_char,
        key_size: usize,
        value_size: *mut usize,
    ) -> *const c_char {
        let key = unsafe { slice::from_raw_parts(key, key_size) };
        let Some(value) = self.cxx_qt_ffi_rust().client.get_variable(index, key) else {
            return ptr::null();
        };
        if !value_size.is_null() {
            *value_size = value.len();
        }
        value.as_ptr()
    }

    /// # Safety
    ///
    /// Refer to the safety documentation for [`std::slice::from_raw_parts`].
    pub unsafe fn set_variable(
        self: Pin<&mut Self>,
        index: PluginIndex,
        key: *const c_char,
        key_size: usize,
        value: *const c_char,
        value_size: usize,
    ) -> bool {
        let key = unsafe { slice::from_raw_parts(key, key_size) }.to_vec();
        let value = unsafe { slice::from_raw_parts(value, value_size) }.to_vec();
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_variable(index, key, value)
    }
}
