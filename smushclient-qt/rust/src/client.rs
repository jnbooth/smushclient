use std::collections::HashMap;
use std::ffi::c_char;
use std::fs::File;
use std::io;
use std::pin::Pin;
use std::ptr;

use crate::adapter::{DocumentAdapter, SocketAdapter, TableBuilderAdapter, TimekeeperAdapter};
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
use mud_transformer::Tag;
use smushclient::world::PersistError;
use smushclient::{
    AliasBool, BoolProperty, Handler, SendIterable, SmushClient, TimerBool, Timers, TriggerBool,
    World,
};
use smushclient_plugins::{Alias, PluginIndex, RegexError, Timer, Trigger};

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
    client: SmushClient,
    input_lock: NonBlockingMutex,
    output_lock: NonBlockingMutex,
    send: Vec<QString>,
    timers: Timers<ffi::SendTimer>,
    palette: HashMap<RgbColor, i32>,
}

impl Default for SmushClientRust {
    fn default() -> Self {
        Self {
            client: SmushClient::new(World::default(), SUPPORTED_TAGS),
            input_lock: NonBlockingMutex::default(),
            output_lock: NonBlockingMutex::default(),
            send: Vec::new(),
            timers: Timers::new(),
            palette: HashMap::with_capacity(166),
        }
    }
}

impl SmushClientRust {
    pub fn load_world(&mut self, path: &QString) -> Result<WorldRust, PersistError> {
        let file = File::open(String::from(path).as_str())?;
        let worldfile = World::load(file)?;
        let world = WorldRust::from(&worldfile);
        self.client.set_world_and_plugins(worldfile);
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
        self.client.set_world(world);
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
        let output_lock = self.output_lock.lock();
        self.send.clear();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            send: &mut self.send,
        };
        let read_result = self.client.read(&mut socket);
        handler.doc.begin();

        self.client.flush_output(&mut handler);
        handler.output_sends();
        handler.doc.end();
        drop(output_lock);

        let total_read = match read_result {
            Ok(total_read) => total_read,
            Err(e) => {
                handler.display_error(&e.to_string());
                return -1;
            }
        };

        let input_lock = self.input_lock.lock();
        if let Err(e) = self.client.write(&mut socket) {
            handler.display_error(&e.to_string());
            return -1;
        };
        drop(input_lock);

        i64::try_from(total_read).unwrap()
    }

    pub fn alias(&mut self, command: &QString, mut doc: DocumentAdapter) -> u8 {
        doc.begin();
        let output_lock = self.output_lock.lock();
        self.send.clear();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            send: &mut self.send,
        };
        let outcome = self.client.alias(&String::from(command), &mut handler);
        handler.output_sends();
        handler.doc.end();
        drop(output_lock);
        convert_alias_outcome(outcome)
    }

    pub fn start_timers(&mut self, index: PluginIndex, mut timekeeper: TimekeeperAdapter) {
        for timer in self.client.senders::<Timer>(index) {
            self.timers.start(index, timer, &mut timekeeper);
        }
    }

    pub fn finish_timer(&mut self, id: usize, mut timekeeper: TimekeeperAdapter) -> bool {
        self.timers.finish(id, &mut self.client, &mut timekeeper)
    }

    fn add_sender<T: SendIterable>(&mut self, index: PluginIndex, sender: T) -> isize {
        match self.client.add_sender(index, sender) {
            Ok((pos, _)) => isize::try_from(pos).unwrap(),
            Err(pos) => -1 - isize::try_from(pos).unwrap(),
        }
    }

    pub fn add_timer(
        &mut self,
        index: PluginIndex,
        timer: Timer,
        mut timekeeper: TimekeeperAdapter,
    ) -> isize {
        let (pos, timer) = match self.client.add_sender(index, timer) {
            Ok((pos, timer)) => (isize::try_from(pos).unwrap(), timer),
            Err(pos) => return -1 - isize::try_from(pos).unwrap(),
        };
        self.timers.start(index, timer, &mut timekeeper);
        pos
    }

    pub fn add_or_replace_timer(
        &mut self,
        index: PluginIndex,
        timer: Timer,
        mut timekeeper: TimekeeperAdapter,
    ) -> usize {
        let (pos, timer) = self.client.add_or_replace_sender(index, timer);
        self.timers.start(index, timer, &mut timekeeper);
        pos
    }

    pub fn set_bool<P>(&mut self, index: PluginIndex, label: &QString, prop: P, value: bool) -> bool
    where
        P: BoolProperty,
        P::Target: SendIterable,
    {
        let Some(sender) = self
            .client
            .find_sender_mut::<P::Target>(index, &String::from(label))
        else {
            return false;
        };
        *prop.get_mut(sender) = value;
        true
    }

    pub fn set_sender_group<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &QString,
        group: &QString,
    ) -> bool {
        let Some(sender) = self
            .client
            .find_sender_mut::<T>(index, &String::from(label))
        else {
            return false;
        };
        sender.as_mut().group = String::from(group);
        true
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

    pub fn open_log(self: Pin<&mut Self>) -> io::Result<()> {
        self.cxx_qt_ffi_rust_mut().client.open_log()
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

    pub fn add_alias(
        self: Pin<&mut Self>,
        index: PluginIndex,
        alias: &ffi::Alias,
    ) -> Result<isize, Box<RegexError>> {
        let alias = Alias::try_from(alias.cxx_qt_ffi_rust())?;
        Ok(self.cxx_qt_ffi_rust_mut().add_sender(index, alias))
    }

    pub fn add_timer(
        self: Pin<&mut Self>,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> isize {
        let timer = Timer::from(timer.cxx_qt_ffi_rust());
        self.cxx_qt_ffi_rust_mut()
            .add_timer(index, timer, timekeeper.into())
    }

    pub fn add_trigger(
        self: Pin<&mut Self>,
        index: PluginIndex,
        trigger: &ffi::Trigger,
    ) -> Result<isize, Box<RegexError>> {
        let trigger = Trigger::try_from(trigger.cxx_qt_ffi_rust())?;
        Ok(self.cxx_qt_ffi_rust_mut().add_sender(index, trigger))
    }

    pub fn remove_alias(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_sender::<Alias>(index, &String::from(name))
            .is_some()
    }

    pub fn remove_timer(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_sender::<Timer>(index, &String::from(name))
            .is_some()
    }

    pub fn remove_trigger(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_sender::<Trigger>(index, &String::from(name))
            .is_some()
    }

    pub fn remove_aliases(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> usize {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_senders::<Alias>(index, &String::from(name))
    }

    pub fn remove_timers(self: Pin<&mut Self>, index: PluginIndex, group: &QString) -> usize {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_senders::<Timer>(index, &String::from(group))
    }

    pub fn remove_triggers(self: Pin<&mut Self>, index: PluginIndex, group: &QString) -> usize {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_senders::<Trigger>(index, &String::from(group))
    }

    pub fn replace_alias(
        self: Pin<&mut Self>,
        index: PluginIndex,
        alias: &ffi::Alias,
    ) -> Result<usize, Box<RegexError>> {
        let alias = Alias::try_from(alias.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_or_replace_sender(index, alias)
            .0)
    }

    pub fn replace_timer(
        self: Pin<&mut Self>,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> usize {
        let timer = Timer::from(timer.cxx_qt_ffi_rust());
        self.cxx_qt_ffi_rust_mut()
            .add_or_replace_timer(index, timer, timekeeper.into())
    }

    pub fn replace_trigger(
        self: Pin<&mut Self>,
        index: PluginIndex,
        trigger: &ffi::Trigger,
    ) -> Result<usize, Box<RegexError>> {
        let trigger = Trigger::try_from(trigger.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_or_replace_sender(index, trigger)
            .0)
    }

    pub fn is_alias(&self, index: PluginIndex, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .find_sender::<Alias>(index, &String::from(label))
            .is_some()
    }

    pub fn is_timer(&self, index: PluginIndex, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .find_sender::<Timer>(index, &String::from(label))
            .is_some()
    }

    pub fn is_trigger(&self, index: PluginIndex, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .find_sender::<Trigger>(index, &String::from(label))
            .is_some()
    }

    pub fn set_alias_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Alias>(index, &String::from(label), enabled)
    }

    pub fn set_aliases_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        group: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Alias>(index, &String::from(group), enabled)
    }

    pub fn set_plugin_enabled(self: Pin<&mut Self>, index: PluginIndex, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_plugin_enabled(index, enabled)
    }

    pub fn set_timer_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Timer>(index, &String::from(label), enabled)
    }

    pub fn set_timers_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        group: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Timer>(index, &String::from(group), enabled)
    }

    pub fn set_trigger_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Trigger>(index, &String::from(label), enabled)
    }

    pub fn set_triggers_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        group: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Trigger>(index, &String::from(group), enabled)
    }

    pub fn set_alias_bool(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        option: ffi::AliasBool,
        value: bool,
    ) -> bool {
        let Ok(option) = AliasBool::try_from(option) else {
            return false;
        };
        self.cxx_qt_ffi_rust_mut()
            .set_bool(index, label, option, value)
    }

    pub fn set_timer_bool(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        option: ffi::TimerBool,
        value: bool,
    ) -> bool {
        let Ok(option) = TimerBool::try_from(option) else {
            return false;
        };
        self.cxx_qt_ffi_rust_mut()
            .set_bool(index, label, option, value)
    }

    pub fn set_trigger_bool(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        option: ffi::TriggerBool,
        value: bool,
    ) -> bool {
        let Ok(option) = TriggerBool::try_from(option) else {
            return false;
        };
        self.cxx_qt_ffi_rust_mut()
            .set_bool(index, label, option, value)
    }

    pub fn set_trigger_group(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        group: &QString,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .set_sender_group::<Trigger>(index, label, group)
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
        key: &[c_char],
        value_size: *mut usize,
    ) -> *const c_char {
        let Some(value) = self.cxx_qt_ffi_rust().client.get_variable(index, key) else {
            return ptr::null();
        };
        if !value_size.is_null() {
            *value_size = value.len();
        }
        value.as_ptr()
    }

    pub fn set_variable(
        self: Pin<&mut Self>,
        index: PluginIndex,
        key: &[c_char],
        value: &[c_char],
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_variable(index, key.to_vec(), value.to_vec())
    }

    pub fn start_timers(
        self: Pin<&mut Self>,
        index: PluginIndex,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) {
        self.cxx_qt_ffi_rust_mut()
            .start_timers(index, timekeeper.into());
    }

    pub fn finish_timer(
        self: Pin<&mut Self>,
        id: usize,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .finish_timer(id, timekeeper.into())
    }
}
