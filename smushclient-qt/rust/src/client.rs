use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::io::{self, Write};
use std::pin::Pin;

use cxx_qt_io::QAbstractSocket;
use cxx_qt_lib::{QColor, QList, QString, QStringList, QVariant};
use mud_transformer::Tag;
use mud_transformer::mxp::RgbColor;
use smushclient::world::PersistError;
use smushclient::{
    AliasOutcome, AudioSinks, BoolProperty, CommandSource, Handler, SendIterable,
    SenderAccessError, SmushClient, Timers, World,
};
use smushclient_plugins::{Alias, LoadError, PluginIndex, Timer, Trigger, XmlError};

use crate::convert::Convert;
use crate::ffi::{self, Document, Timekeeper};
use crate::get_info::InfoVisitorQVariant;
use crate::handler::ClientHandler;
use crate::modeled::Modeled;
use crate::world::WorldRust;

const BUF_LEN: usize = 1024 * 20;

pub struct SmushClientRust {
    audio: AudioSinks,
    pub client: SmushClient,
    read_buf: RefCell<Vec<u8>>,
    stats: RefCell<HashSet<String>>,
    timers: RefCell<Timers<ffi::SendTimer>>,
    palette: HashMap<RgbColor, i32>,
}

impl Default for SmushClientRust {
    /// # Panics
    ///
    /// Panics if audio initialization fails.
    #[allow(clippy::expect_used)]
    fn default() -> Self {
        Self {
            audio: AudioSinks::try_default().expect("audio initialization failed"),
            read_buf: RefCell::new(vec![0; BUF_LEN]),
            stats: RefCell::new(HashSet::new()),
            timers: RefCell::new(Timers::new()),
            palette: HashMap::with_capacity(164),
            client: SmushClient::new(
                World::default(),
                Tag::Bold
                    | Tag::Color
                    | Tag::Expire
                    | Tag::Font
                    | Tag::H1
                    | Tag::H2
                    | Tag::H3
                    | Tag::H4
                    | Tag::H5
                    | Tag::H6
                    | Tag::Highlight
                    | Tag::Hr
                    | Tag::Hyperlink
                    | Tag::Italic
                    | Tag::Send
                    | Tag::Strikeout
                    | Tag::Underline,
            ),
        }
    }
}

#[allow(clippy::cast_possible_truncation)]
#[allow(clippy::cast_possible_wrap)]
impl SmushClientRust {
    pub fn load_world(&mut self, path: &QString) -> Result<WorldRust, PersistError> {
        let file = File::open(String::from(path).as_str())?;
        let worldfile = World::load(file)?;
        let world = WorldRust::from(&worldfile);
        self.client.set_world(worldfile);
        self.apply_world();
        Ok(world)
    }

    pub fn load_plugins(&mut self) -> QStringList {
        let Err(errors) = self.client.load_plugins() else {
            return QStringList::default();
        };
        let mut list: QList<QString> = QList::default();
        list.reserve(2 * errors.len() as isize);
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

    pub fn load_variables(&self, path: &QString) -> Result<bool, PersistError> {
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

    pub fn set_world(&mut self, world: &WorldRust) -> io::Result<bool> {
        let Ok(world) = World::try_from(world) else {
            return Ok(false);
        };
        self.apply_world();
        self.client.update_world(world)
    }

    pub fn handle_connect(&self, mut socket: Pin<&mut QAbstractSocket>) -> QString {
        let connect_message = self.client.world().connect_message();
        match socket.write_all(connect_message.as_bytes()) {
            Ok(()) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        }
    }

    pub fn handle_disconnect(&mut self) {
        self.stats.borrow_mut().clear();
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
            .expect("world plugin is missing!")
    }

    /// # Panics
    ///
    /// Panics if the index is out of bounds.
    #[track_caller]
    pub fn plugin(&self, index: PluginIndex) -> ffi::PluginPack {
        #[cold]
        #[track_caller]
        fn assert_failed(index: PluginIndex, len: usize) -> ! {
            panic!("plugin index (is {index}) should be < len (is {len})");
        }
        match self.client.plugin(index) {
            Some(plugin) => plugin.into(),
            None => assert_failed(index, self.client.plugins_len()),
        }
    }

    pub fn reset_world_plugin(&self) {
        self.timers
            .borrow_mut()
            .reset_plugin(self.world_plugin_index());
        self.client.reset_world_plugin();
    }

    pub fn reset_plugins(&self) -> Vec<ffi::PluginPack> {
        self.client.reset_plugins();
        self.timers.borrow_mut().clear();
        self.client.plugins().map(ffi::PluginPack::from).collect()
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        self.timers.borrow_mut().reset_plugin(index);
        self.client.reinstall_plugin(index)
    }

    pub fn plugin_model_text(&self, index: PluginIndex, column: i32) -> QString {
        let Some(plugin) = self.client.plugin(index) else {
            return QString::default();
        };
        plugin.cell_text(column)
    }

    fn apply_world(&mut self) {
        self.palette.clear();
        for (i, color) in self.client.world().palette().iter().enumerate() {
            self.palette.insert(*color, i as i32);
        }
    }

    pub fn read(&mut self, mut socket: Pin<&mut QAbstractSocket>, doc: Pin<&mut Document>) -> i64 {
        let world = self.client.world();
        let mut handler = ClientHandler {
            audio: &self.audio,
            doc,
            palette: &self.palette,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats: &self.stats,
        };
        let read_result = self
            .client
            .read(&mut socket, &mut self.read_buf.borrow_mut());
        handler.doc.begin();

        let had_output = self.client.drain_output(&mut handler);
        handler.doc.as_mut().end(had_output);

        let total_read = match read_result {
            Ok(total_read) => total_read,
            Err(e) => {
                handler.display_error(&e.to_string());
                return -1;
            }
        };

        if let Err(e) = self.client.write(&mut socket) {
            handler.display_error(&e.to_string());
            return -1;
        }

        total_read as i64
    }

    pub fn flush(&mut self, doc: Pin<&mut Document>) {
        let world = self.client.world();
        let mut handler = ClientHandler {
            audio: &self.audio,
            doc,
            palette: &self.palette,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats: &self.stats,
        };
        handler.doc.begin();

        let had_output = self.client.flush_output(&mut handler);
        handler.doc.end(had_output);
    }

    pub fn play_buffer(
        &self,
        i: usize,
        buf: &[u8],
        volume: f32,
        looping: bool,
    ) -> ffi::SoundResult {
        self.audio
            .play_buffer(i, buf.to_vec(), volume, looping.into())
            .into()
    }

    pub fn play_file(
        &self,
        i: usize,
        path: &QString,
        volume: f32,
        looping: bool,
    ) -> ffi::SoundResult {
        let looping = looping.into();
        if path.is_empty() {
            return self.audio.configure_sink(i, volume, looping).into();
        }
        self.audio
            .play_file(i, String::from(path), volume, looping)
            .into()
    }

    pub fn stop_sound(&self, i: usize) -> ffi::SoundResult {
        self.audio.stop(i).into()
    }

    pub fn alias(
        &self,
        command: &QString,
        source: CommandSource,
        doc: Pin<&mut Document>,
    ) -> AliasOutcome {
        let world = self.client.world();
        let mut handler = ClientHandler {
            audio: &self.audio,
            doc,
            palette: &self.palette,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats: &self.stats,
        };
        handler.doc.begin();
        let outcome = self
            .client
            .alias(&String::from(command), source, &mut handler);
        handler.doc.end(false);
        outcome
    }

    pub fn timer_info(&self, index: PluginIndex, label: &QString, info_type: u8) -> QVariant {
        self.client.timer_info::<InfoVisitorQVariant, _>(
            index,
            &String::from(label),
            info_type,
            &self.timers.borrow(),
        )
    }

    pub fn start_timers(&self, index: PluginIndex, mut timekeeper: Pin<&mut Timekeeper>) {
        if !self.client.world().enable_timers {
            return;
        }
        let timer_starts = {
            let mut timers = self.timers.borrow_mut();
            self.client
                .senders::<Timer>(index)
                .borrow()
                .iter()
                .filter_map(|timer| timers.start(index, timer))
                .collect::<Vec<_>>()
        };
        for timer_start in &timer_starts {
            timekeeper.as_mut().start(timer_start);
        }
    }

    pub fn start_all_timers(&self, mut timekeeper: Pin<&mut Timekeeper>) {
        if !self.client.world().enable_timers {
            return;
        }
        let mut timer_starts = Vec::new();
        {
            let mut timers = self.timers.borrow_mut();
            for index in 0..self.client.plugins_len() {
                timer_starts.extend(
                    self.client
                        .senders::<Timer>(index)
                        .borrow()
                        .iter()
                        .filter_map(|timer| timers.start(index, timer)),
                );
            }
        };
        for timer_start in &timer_starts {
            timekeeper.as_mut().start(timer_start);
        }
    }

    pub fn finish_timer(&self, id: usize, timekeeper: &Timekeeper) -> bool {
        let result = self.timers.borrow_mut().finish(id, &self.client);
        if let Some(timer) = result.timer {
            timekeeper.send_timer(&timer);
        }
        result.done
    }

    pub fn poll_timers(&self, timekeeper: &Timekeeper) {
        for timer in self.timers.borrow_mut().poll(&self.client) {
            timekeeper.send_timer(&timer);
        }
    }

    pub fn stop_senders(&self) {
        self.client.stop_evaluating::<Alias>();
        self.client.stop_evaluating::<Timer>();
        self.client.stop_evaluating::<Trigger>();
    }

    pub fn add_timer(
        &self,
        index: PluginIndex,
        timer: Timer,
        timekeeper: Pin<&mut Timekeeper>,
    ) -> Result<(), SenderAccessError> {
        let enable_timers = self.client.world().enable_timers;
        let start = {
            let timer = self.client.add_sender(index, timer)?;
            if !enable_timers {
                return Ok(());
            }
            self.timers.borrow_mut().start(index, &timer)
        };
        if let Some(start) = start {
            timekeeper.start(&start);
        }
        Ok(())
    }

    pub fn add_or_replace_timer(
        &self,
        index: PluginIndex,
        timer: Timer,
        timekeeper: Pin<&mut Timekeeper>,
    ) {
        let enable_timers = self.client.world().enable_timers;
        let start = {
            let timer = self.client.add_or_replace_sender(index, timer);
            if !enable_timers {
                return;
            }
            self.timers.borrow_mut().start(index, &timer)
        };
        if let Some(start) = start {
            timekeeper.start(&start);
        }
    }

    pub fn replace_world_timer(
        &self,
        index: usize,
        timer: Timer,
        timekeeper: Pin<&mut Timekeeper>,
    ) -> Result<(), SenderAccessError> {
        let enable_timers = self.client.world().enable_timers;
        let world_index = self.world_plugin_index();
        let start = {
            let (_, timer) = self.client.replace_world_sender(index, timer)?;
            if !enable_timers {
                return Ok(());
            }
            self.timers.borrow_mut().start(index, &timer)
        };
        if let Some(mut start) = start {
            start.index = world_index;
            timekeeper.start(&start);
        }
        Ok(())
    }

    pub fn import_world_timers(
        &self,
        xml: &QString,
        mut timekeeper: Pin<&mut Timekeeper>,
    ) -> Result<(), XmlError> {
        let enable_timers = self.client.world().enable_timers;
        let world_index = self.world_plugin_index();
        let timers = self
            .client
            .import_world_senders::<Timer>(&String::from(xml))?;
        if enable_timers {
            for timer in &*timers {
                if let Some(start) = self.timers.borrow_mut().start(world_index, timer) {
                    timekeeper.as_mut().start(&start);
                }
            }
        }
        Ok(())
    }

    pub fn set_bool<P>(
        &self,
        index: PluginIndex,
        label: &QString,
        prop: P,
        value: bool,
    ) -> Result<(), SenderAccessError>
    where
        P: BoolProperty,
        P::Target: SendIterable,
    {
        let mut sender = self
            .client
            .borrow_sender_mut::<P::Target>(index, &String::from(label))?;
        *prop.get_mut(&mut sender) = value;
        Ok(())
    }

    pub fn set_sender_group<T: SendIterable>(
        &self,
        index: PluginIndex,
        label: &QString,
        group: &QString,
    ) -> Result<(), SenderAccessError> {
        let mut sender = self
            .client
            .borrow_sender_mut::<T>(index, &String::from(label))?;
        sender.as_mut().group = String::from(group);
        Ok(())
    }
}
