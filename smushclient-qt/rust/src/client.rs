use std::cell::RefCell;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, Write};
use std::path::Path;
use std::pin::Pin;

use cxx_qt::casting::Downcast;
use cxx_qt_io::{QAbstractSocket, QIODevice, QNetworkProxy, QNetworkProxyProxyType, QSslSocket};
use cxx_qt_lib::{QString, QStringList, QVariant};
use mud_transformer::Tag;
use smushclient::world::PersistError;
use smushclient::{
    AliasOutcome, CommandSource, Handler, LuaStr, OptionError, Optionable, SendIterable,
    SenderAccessError, SmushClient, Timers, World,
};
use smushclient_plugins::{Alias, LoadError, PluginIndex, Timer, Trigger, XmlError};

use crate::convert::Convert;
use crate::ffi::{self, AliasMenuItem, Document, Timekeeper};
use crate::get_info::InfoVisitorQVariant;
use crate::handler::ClientHandler;
use crate::results::IntoApiCode;
use crate::text_formatter::TextFormatter;
use crate::world::WorldRust;

const BUF_LEN: usize = 1024 * 20;

pub struct SmushClientRust {
    pub client: SmushClient,
    read_buf: Vec<u8>,
    stats: RefCell<HashSet<String>>,
    timers: RefCell<Timers<ffi::SendTimer>>,
    formatter: TextFormatter,
}

impl Default for SmushClientRust {
    /// # Panics
    ///
    /// Panics if audio initialization fails.
    fn default() -> Self {
        Self {
            read_buf: vec![0; BUF_LEN],
            stats: RefCell::new(HashSet::new()),
            timers: RefCell::new(Timers::new()),
            formatter: TextFormatter::default(),
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
    pub fn load_world<P: AsRef<Path>>(&mut self, path: P) -> Result<(), PersistError> {
        let file = File::open(path)?;
        let world = World::load(file)?;
        self.client.set_world(world);
        self.apply_world();
        Ok(())
    }

    pub fn load_plugins(&mut self) -> QStringList {
        let Err(errors) = self.client.load_plugins() else {
            return QStringList::default();
        };
        let mut list: QStringList = QStringList::default();
        list.reserve(2 * errors.len() as isize);
        for error in &errors {
            list.append(QString::from(&*error.path.to_string_lossy()));
            list.append(QString::from(&error.error.to_string()));
        }
        list
    }

    pub fn save_world<P: AsRef<Path>>(&self, path: P) -> Result<(), PersistError> {
        let file = File::create(path)?;
        self.client.world().save(file)
    }

    pub fn load_variables<P: AsRef<Path>>(&self, path: P) -> Result<bool, PersistError> {
        let file = match File::open(path) {
            Err(err) if err.kind() == io::ErrorKind::NotFound => return Ok(false),
            file => file?,
        };
        self.client.load_variables(file)?;
        Ok(true)
    }

    pub fn save_variables<P: AsRef<Path>>(&self, path: P) -> Result<bool, PersistError> {
        if !self.client.has_variables() {
            return Ok(false);
        }
        let file = File::create(path)?;
        self.client.save_variables(file)?;
        Ok(true)
    }

    pub fn set_world(&mut self, world: &WorldRust) -> io::Result<bool> {
        let Ok(world) = World::try_from(world) else {
            return Ok(false);
        };
        self.apply_world();
        self.client.update_world(world)
    }

    pub fn get_world(&self) -> WorldRust {
        WorldRust::from(self.client.world())
    }

    pub fn connect_to_host(&self, mut socket: Pin<&mut QAbstractSocket>) {
        let world = self.client.world();
        if world.use_proxy {
            let mut proxy = QNetworkProxy::default();
            proxy.set_host_name(&QString::from(&world.proxy_server));
            proxy.set_port(world.proxy_port);
            proxy.set_user(&QString::from(&world.proxy_username));
            proxy.set_password(&QString::from(&world.proxy_password));
            proxy.set_proxy_type(QNetworkProxyProxyType::Socks5Proxy);
            socket.as_mut().set_proxy(&proxy);
        } else {
            socket
                .as_mut()
                .set_proxy(&QNetworkProxyProxyType::NoProxy.into());
        }
        if world.use_ssl
            && QSslSocket::supports_ssl()
            && let Some(socket) = socket.as_mut().downcast_pin::<QSslSocket>()
        {
            socket.connect_to_host_encrypted((&world.site, world.port), QIODevice::ReadWrite);
        } else {
            socket.connect_to_host((&world.site, world.port), QIODevice::ReadWrite);
        }
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

    fn handler<'a>(&'a self, doc: Pin<&'a mut Document>) -> ClientHandler<'a> {
        let world = self.client.world();
        ClientHandler {
            doc,
            formatter: &self.formatter,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats: &self.stats,
        }
    }

    pub fn simulate(&self, line: &str, doc: Pin<&mut Document>) {
        let mut handler = self.handler(doc);
        self.client.simulate_output(line, &mut handler);
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

    fn apply_world(&mut self) {
        self.formatter.apply_world(self.client.world());
    }

    pub fn read(&mut self, mut socket: Pin<&mut QAbstractSocket>, doc: Pin<&mut Document>) -> i64 {
        let world = self.client.world();
        let mut handler = ClientHandler {
            doc,
            formatter: &self.formatter,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats: &self.stats,
        };
        let read_result = self.client.read(&mut socket, &mut self.read_buf);
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
            doc,
            formatter: &self.formatter,
            carriage_return_clears_line: world.carriage_return_clears_line,
            no_echo_off: world.no_echo_off,
            stats: &self.stats,
        };
        handler.doc.begin();

        let had_output = self.client.flush_output(&mut handler);
        handler.doc.end(had_output);
    }

    pub fn handle_alert(&self) -> ffi::ApiCode {
        let sound = &self.client.world().new_activity_sound;
        if sound.is_empty() {
            return ffi::ApiCode::OK;
        }
        self.client.play_file_raw(sound).code()
    }

    pub fn alias(
        &self,
        command: &str,
        source: CommandSource,
        doc: Pin<&mut Document>,
    ) -> AliasOutcome {
        let mut handler = self.handler(doc);
        handler.doc.begin();
        let outcome = self.client.alias(command, source, &mut handler);
        handler.doc.end(false);
        outcome
    }

    pub fn invoke_alias(&self, index: PluginIndex, id: u16, doc: Pin<&mut Document>) -> bool {
        let mut handler = self.handler(doc);
        handler.doc.begin();
        let succeeded = self.client.invoke_alias(index, id, &mut handler);
        handler.doc.end(false);
        succeeded
    }

    pub fn alias_menu(&self) -> Vec<AliasMenuItem> {
        let mut menu = Vec::new();
        self.client.build_alias_menu(|plugin, id, label| {
            menu.push(AliasMenuItem {
                plugin,
                id,
                text: QString::from(label),
            });
        });
        menu
    }

    pub fn timer_info(&self, index: PluginIndex, label: &str, info_type: u8) -> QVariant {
        self.client.timer_info::<InfoVisitorQVariant, _>(
            index,
            label,
            info_type,
            &self.timers.borrow(),
        )
    }

    pub fn start_timers(&self, index: PluginIndex, timekeeper: &Timekeeper) {
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
            timekeeper.start(timer_start);
        }
    }

    pub fn start_all_timers(&self, timekeeper: &Timekeeper) {
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
            timekeeper.start(timer_start);
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
        timekeeper: &Timekeeper,
    ) -> Result<(), SenderAccessError> {
        let start = {
            let timer = self.client.add_sender(index, timer)?;
            if !self.client.world().enable_timers {
                return Ok(());
            }
            self.timers.borrow_mut().start(index, &timer)
        };
        if let Some(start) = start {
            timekeeper.start(&start);
        }
        Ok(())
    }

    pub fn add_or_replace_timer(&self, index: PluginIndex, timer: Timer, timekeeper: &Timekeeper) {
        let start = {
            let timer = self.client.add_or_replace_sender(index, timer);
            if !self.client.world().enable_timers {
                return;
            }
            self.timers.borrow_mut().start(index, &timer)
        };
        if let Some(start) = start {
            timekeeper.start(&start);
        }
    }

    pub fn import_world_timers(&self, xml: &str, timekeeper: &Timekeeper) -> Result<(), XmlError> {
        let world_index = self.world_plugin_index();
        let imported_timers = self.client.import_world_senders::<Timer>(xml)?;
        if !self.client.world().enable_timers {
            return Ok(());
        }
        let mut timers = self.timers.borrow_mut();
        for timer in &imported_timers {
            if let Some(start) = timers.start(world_index, timer) {
                timekeeper.start(&start);
            }
        }
        Ok(())
    }

    pub fn replace_world_alias(
        &self,
        index: usize,
        alias: Alias,
    ) -> Result<usize, ffi::ReplaceSenderResult> {
        let group = alias.group.clone();
        let (i, alias) = self.client.replace_world_sender(index, alias)?;
        if alias.group == group {
            Ok(i)
        } else {
            Err(ffi::ReplaceSenderResult::GroupChanged)
        }
    }

    pub fn replace_world_trigger(
        &self,
        index: usize,
        trigger: Trigger,
    ) -> Result<usize, ffi::ReplaceSenderResult> {
        let group = trigger.group.clone();
        let (i, trigger) = self.client.replace_world_sender(index, trigger)?;
        if trigger.group == group {
            Ok(i)
        } else {
            Err(ffi::ReplaceSenderResult::GroupChanged)
        }
    }

    pub fn replace_world_timer(
        &self,
        index: usize,
        timer: Timer,
        timekeeper: &Timekeeper,
    ) -> Result<usize, ffi::ReplaceSenderResult> {
        let group = timer.group.clone();
        let (result, start) = {
            let (i, timer) = self.client.replace_world_sender(index, timer)?;
            let result = if timer.group == group {
                Ok(i)
            } else {
                Err(ffi::ReplaceSenderResult::GroupChanged)
            };
            if !self.client.world().enable_timers {
                return result;
            }
            let world_index = self.world_plugin_index();
            (result, self.timers.borrow_mut().start(world_index, &timer))
        };
        if let Some(start) = start {
            timekeeper.start(&start);
        }
        result
    }

    pub fn get_sender_option<T: SendIterable + Optionable>(
        &self,
        index: PluginIndex,
        label: &str,
        option: &LuaStr,
    ) -> QVariant {
        let Some(sender) = self.client.borrow_sender::<T>(index, label) else {
            return QVariant::default();
        };
        sender.get_option(option).convert()
    }

    pub fn set_sender_option<T: SendIterable + Optionable>(
        &self,
        index: PluginIndex,
        label: &str,
        option: &LuaStr,
        value: &LuaStr,
    ) -> Result<(), OptionError> {
        self.client
            .borrow_sender_mut::<T>(index, label)?
            .set_option(option, value)
    }
}
