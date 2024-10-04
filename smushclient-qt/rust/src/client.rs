use std::collections::HashMap;
use std::ffi::c_char;
use std::fs::File;
use std::io::{self, Read};
use std::pin::Pin;
use std::{ptr, slice};

use crate::adapter::{DocumentAdapter, SocketAdapter, TableBuilderAdapter};
use crate::convert::Convert;
use crate::ffi;
use crate::get_info::InfoVisitorQVariant;
use crate::handler::ClientHandler;
use crate::impls::convert_alias_outcome;
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
        self.transformer
            .set_config(self.client.set_world(worldfile));
        self.apply_world();
        Ok(world)
    }

    pub fn load_plugins(&mut self) -> QStringList {
        let Err(errors) = self.client.load_plugins() else {
            return QStringList::default();
        };
        let mut list: QList<QString> = QList::default();
        list.reserve(isize::try_from(errors.len() * 2).unwrap());
        for error in &errors {
            list.append(QString::from(&*error.path.to_string_lossy()));
            list.append(QString::from(&error.error.to_string()));
        }
        QStringList::from(&list)
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
        self.transformer
            .set_config(self.client.set_world_with_plugins(world));
        self.apply_world();
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

    pub fn build_plugins_table(&self, mut table: TableBuilderAdapter) {
        let plugins = self.client.plugins();
        table.set_row_count(i32::try_from(plugins.len()).unwrap());
        for plugin in plugins {
            let metadata = &plugin.metadata;
            if metadata.is_world_plugin {
                continue;
            }
            table.start_row(&QString::from(&metadata.id));
            table.add_column(&QString::from(&metadata.name));
            table.add_column(&QString::from(&metadata.purpose));
            table.add_column(&QString::from(&metadata.author));
            table.add_column(&QString::from(&*metadata.path.to_string_lossy()));
            table.add_column(!plugin.disabled);
            table.add_column(&QString::from(&metadata.version));
        }
    }

    fn apply_world(&mut self) {
        let world = self.client.world();
        self.palette.clear();
        for (i, color) in world.palette().iter().enumerate() {
            self.palette.insert(*color, i32::try_from(i).unwrap());
        }
    }

    pub fn read(&mut self, mut socket: SocketAdapter, doc: DocumentAdapter) -> i64 {
        if self.done {
            return -1;
        }
        let output_lock = self.output_lock.lock();
        self.send.clear();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            send: &mut self.send,
        };
        let mut total_read = 0;
        loop {
            let n = match socket.read(&mut self.buf[..BUF_MIDPOINT]) {
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
            if let Err(e) = drain.write_all_to(&mut socket) {
                handler.display_error(&e.to_string());
                return -1;
            }
        }
        drop(input_lock);

        i64::try_from(total_read).unwrap()
    }

    pub fn alias(&mut self, command: &QString, doc: DocumentAdapter) -> u8 {
        let output_lock = self.output_lock.lock();
        self.send.clear();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            send: &mut self.send,
        };
        let outcome = self.client.alias(&String::from(command), &mut handler);
        handler.output_sends();
        handler.doc.scroll_to_bottom();
        drop(output_lock);
        convert_alias_outcome(outcome)
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

    pub fn load_plugins(self: Pin<&mut Self>) -> QStringList {
        self.cxx_qt_ffi_rust_mut().load_plugins()
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

    pub fn build_plugins_table(&self, table: Pin<&mut ffi::TableBuilder>) {
        self.cxx_qt_ffi_rust().build_plugins_table(table.into());
    }

    pub fn add_plugin(self: Pin<&mut Self>, path: &QString) -> QString {
        match self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_plugin(&String::from(path))
        {
            Ok(_) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        }
    }

    pub fn remove_plugin(self: Pin<&mut Self>, plugin_id: &QString) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_plugin(&String::from(plugin_id))
            .is_some()
    }

    pub fn plugin_scripts(&self) -> QStringList {
        self.cxx_qt_ffi_rust().plugin_scripts()
    }

    pub fn read(
        self: Pin<&mut Self>,
        device: Pin<&mut ffi::QTcpSocket>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device.into(), doc.into())
    }

    pub fn is_alias(&self, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .sender_exists::<Alias>(&String::from(label))
    }

    pub fn is_timer(&self, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .sender_exists::<Timer>(&String::from(label))
    }

    pub fn is_trigger(&self, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .sender_exists::<Trigger>(&String::from(label))
    }

    pub fn set_alias_enabled(self: Pin<&mut Self>, label: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Alias>(&String::from(label), enabled)
    }

    pub fn set_aliases_enabled(self: Pin<&mut Self>, group: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Alias>(&String::from(group), enabled)
    }

    pub fn set_plugin_enabled(self: Pin<&mut Self>, index: PluginIndex, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_plugin_enabled(index, enabled)
    }

    pub fn set_timer_enabled(self: Pin<&mut Self>, label: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Timer>(&String::from(label), enabled)
    }

    pub fn set_timers_enabled(self: Pin<&mut Self>, group: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Timer>(&String::from(group), enabled)
    }

    pub fn set_trigger_enabled(self: Pin<&mut Self>, label: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Trigger>(&String::from(label), enabled)
    }

    pub fn set_triggers_enabled(self: Pin<&mut Self>, group: &QString, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Trigger>(&String::from(group), enabled)
    }

    pub fn alias(self: Pin<&mut Self>, command: &QString, doc: Pin<&mut ffi::Document>) -> u8 {
        self.cxx_qt_ffi_rust_mut().alias(command, doc.into())
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
