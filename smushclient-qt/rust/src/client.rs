use std::collections::HashMap;
use std::fs::File;
use std::io;
use std::io::Write;

use crate::adapter::{
    DocumentAdapter, ModelBuilderAdapter, RowInsertable, SocketAdapter, TimekeeperAdapter,
};
use crate::bridge::AliasOutcomes;
use crate::convert::Convert;
use crate::ffi;
use crate::get_info::InfoVisitorQVariant;
use crate::handler::ClientHandler;
use crate::sync::NonBlockingMutex;
use crate::world::WorldRust;
use cxx_qt_lib::{QColor, QList, QString, QStringList, QVariant};
use enumeration::EnumSet;
use mud_transformer::mxp::RgbColor;
use mud_transformer::Tag;
use smushclient::world::PersistError;
use smushclient::{
    BoolProperty, CommandSource, Handler, SendIterable, SenderAccessError, SmushClient, Timers,
    World,
};
use smushclient_plugins::{Alias, PluginIndex, Timer, Trigger, XmlError};

const SUPPORTED_TAGS: EnumSet<Tag> = enums![
    Tag::Bold,
    Tag::Color,
    Tag::Expire,
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
    pub client: SmushClient,
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
            palette: HashMap::with_capacity(164),
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
        let Ok(mut world) = World::try_from(world) else {
            return false;
        };
        world.swap_senders(self.client.world_mut());
        self.client.set_world(world);
        self.apply_world();
        true
    }

    pub fn handle_connect(&self, mut socket: SocketAdapter) -> QString {
        let input_lock = self.input_lock.lock();
        let connect_message = self.client.world().connect_message();
        let error = match socket.write_all(connect_message.as_bytes()) {
            Ok(()) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        };
        drop(input_lock);
        error
    }

    pub fn handle_disconnect(&mut self) {
        self.client.reset_connection();
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

    pub fn world_plugin_index(&self) -> PluginIndex {
        self.client
            .plugins()
            .position(|plugin| plugin.metadata.is_world_plugin)
            .unwrap()
    }

    pub fn plugin_scripts(&self) -> Vec<ffi::PluginPack> {
        self.client
            .plugins()
            .map(|plugin| ffi::PluginPack {
                id: QString::from(&plugin.metadata.id),
                name: QString::from(&plugin.metadata.name),
                path: QString::from(&*plugin.metadata.path.to_string_lossy()),
                scriptData: plugin.script.as_ptr(),
                scriptSize: plugin.script.len(),
            })
            .collect()
    }

    pub fn build_plugins_table(&self, mut builder: ModelBuilderAdapter) -> usize {
        let plugins = self.client.plugins();
        let mut count = 0;
        for plugin in plugins {
            let metadata = &plugin.metadata;
            if metadata.is_world_plugin {
                continue;
            }
            count += 1;
            builder.add_row(&QString::from(&metadata.id), plugin);
        }
        count
    }

    pub fn build_senders_tree<T: SendIterable + RowInsertable>(
        &self,
        mut builder: ModelBuilderAdapter,
        group: bool,
    ) -> usize {
        let senders = T::from_world(self.client.world());
        if !group {
            for (index, item) in senders.iter().enumerate() {
                builder.add_row(&u64::try_from(index).unwrap_or(u64::MAX), item);
            }
            return senders.len();
        }
        let mut sorted_items: Vec<(usize, &T)> = senders.iter().enumerate().collect();
        sorted_items.sort_unstable_by_key(|(_, item)| (*item).as_ref());
        let mut last_group = "";
        for (index, item) in sorted_items {
            let group = item.as_ref().group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.add_row(&u64::try_from(index).unwrap_or(u64::MAX), item);
        }
        senders.len()
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
        let world = self.client.world();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
        };
        let read_result = self.client.read(&mut socket);
        handler.doc.begin();

        self.client.drain_output(&mut handler);
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

    pub fn flush(&mut self, doc: DocumentAdapter) {
        let output_lock = self.output_lock.lock();
        self.send.clear();
        let world = self.client.world();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
        };
        handler.doc.begin();

        self.client.flush_output(&mut handler);
        handler.doc.end();
        drop(output_lock);
    }

    pub fn alias(
        &mut self,
        command: &QString,
        source: CommandSource,
        doc: DocumentAdapter,
    ) -> AliasOutcomes {
        doc.begin();
        let output_lock = self.output_lock.lock();
        self.send.clear();
        let world = self.client.world();
        let mut handler = ClientHandler {
            doc,
            palette: &self.palette,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
        };
        let outcome = self
            .client
            .alias(&String::from(command), source, &mut handler);
        handler.doc.end();
        drop(output_lock);
        outcome.into()
    }

    pub fn start_timers(&mut self, index: PluginIndex, mut timekeeper: TimekeeperAdapter) {
        if !self.client.world().enable_timers {
            return;
        }
        for timer in self.client.senders::<Timer>(index) {
            self.timers.start(index, timer, &mut timekeeper);
        }
    }

    pub fn finish_timer(&mut self, id: usize, mut timekeeper: TimekeeperAdapter) -> bool {
        self.timers.finish(id, &mut self.client, &mut timekeeper)
    }

    pub fn poll_timers(&mut self, mut timekeeper: TimekeeperAdapter) {
        self.timers.poll(&mut self.client, &mut timekeeper);
    }

    pub fn stop_senders(&mut self) {
        self.client.stop_evaluating::<Alias>();
        self.client.stop_evaluating::<Timer>();
        self.client.stop_evaluating::<Trigger>();
    }

    pub fn add_timer(
        &mut self,
        index: PluginIndex,
        timer: Timer,
        mut timekeeper: TimekeeperAdapter,
    ) -> Result<(), SenderAccessError> {
        let enable_timers = self.client.world().enable_timers;
        let timer = self.client.add_sender(index, timer)?;
        if enable_timers {
            self.timers.start(index, timer, &mut timekeeper);
        }
        Ok(())
    }

    pub fn add_or_replace_timer(
        &mut self,
        index: PluginIndex,
        timer: Timer,
        mut timekeeper: TimekeeperAdapter,
    ) {
        let enable_timers = self.client.world().enable_timers;
        let timer = self.client.add_or_replace_sender(index, timer);
        if enable_timers {
            self.timers.start(index, timer, &mut timekeeper);
        }
    }

    pub fn replace_world_timer(
        &mut self,
        index: usize,
        timer: Timer,
        mut timekeeper: TimekeeperAdapter,
    ) -> Result<(), SenderAccessError> {
        let enable_timers = self.client.world().enable_timers;
        let world_index = self.world_plugin_index();
        let timer = self.client.replace_world_sender(index, timer)?;
        if enable_timers {
            self.timers.start(world_index, timer, &mut timekeeper);
        }
        Ok(())
    }

    pub fn import_world_timers(
        &mut self,
        xml: &QString,
        mut timekeeper: TimekeeperAdapter,
    ) -> Result<(), XmlError> {
        let enable_timers = self.client.world().enable_timers;
        let world_index = self.world_plugin_index();
        let timers = self
            .client
            .import_world_senders::<Timer>(&String::from(xml))?;
        if enable_timers {
            for timer in &timers {
                self.timers.start(world_index, timer, &mut timekeeper);
            }
        }
        Ok(())
    }

    pub fn set_bool<P>(
        &mut self,
        index: PluginIndex,
        label: &QString,
        prop: P,
        value: bool,
    ) -> Result<(), SenderAccessError>
    where
        P: BoolProperty,
        P::Target: SendIterable,
    {
        let sender = self
            .client
            .find_sender_mut::<P::Target>(index, &String::from(label))?;
        *prop.get_mut(sender) = value;
        Ok(())
    }

    pub fn set_sender_group<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &QString,
        group: &QString,
    ) -> Result<(), SenderAccessError> {
        let sender = self
            .client
            .find_sender_mut::<T>(index, &String::from(label))?;
        sender.as_mut().group = String::from(group);
        Ok(())
    }
}
