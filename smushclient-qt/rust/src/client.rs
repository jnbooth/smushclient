use std::cell::RefCell;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, Write};
use std::path::Path;
use std::pin::Pin;
use std::rc::Rc;

use cxx_qt::casting::Downcast;
use cxx_qt_io::{QAbstractSocket, QIODevice, QNetworkProxy, QNetworkProxyProxyType, QSslSocket};
use cxx_qt_lib::{QString, QStringList, QVariant};
use flagset::FlagSet;
use mud_transformer::Tag;
use smushclient::world::PersistError;
use smushclient::{
    AliasOutcome, CommandSource, Handler, SmushClient, TimerFinish, TimerStart, Timers, World,
    WorldConfig,
};
use smushclient_plugins::{ImportError, LoadError, PluginIndex, SenderAccessError, Timer};

use crate::ffi;
use crate::get_info::InfoVisitorQVariant;
use crate::handler::ClientHandler;
use crate::text_formatter::TextFormatter;

const BUF_LEN: usize = 1024 * 20;

pub struct SmushClientRust {
    pub client: SmushClient,
    read_buf: RefCell<Box<[u8]>>,
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
            read_buf: RefCell::new(vec![0; BUF_LEN].into_boxed_slice()),
            stats: RefCell::new(HashSet::new()),
            timers: RefCell::new(Timers::new()),
            formatter: TextFormatter::default(),
            client: SmushClient::new(World::default(), Self::supported_tags()),
        }
    }
}

impl SmushClientRust {
    fn supported_tags() -> FlagSet<Tag> {
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
            | Tag::Underline
    }

    // World

    fn apply_world(&mut self, world: &WorldConfig) {
        self.formatter.apply_world(world);
    }

    pub fn load_world<P: AsRef<Path>>(&mut self, path: P) -> Result<(), PersistError> {
        let world = World::load(File::open(path)?)?;
        self.apply_world(&world.config);
        self.client = SmushClient::new(world, Self::supported_tags());
        Ok(())
    }

    pub fn import_world<P: AsRef<Path>>(&mut self, path: P) -> Result<(), ImportError> {
        let file = File::open(path)?;
        let client = SmushClient::import_world(file, Self::supported_tags())?;
        self.apply_world(&client.borrow_world());
        self.client = client;
        Ok(())
    }

    pub fn set_world(&mut self, world: WorldConfig) -> bool {
        self.apply_world(&world);
        self.client.update_world(world)
    }

    pub fn save_world<P: AsRef<Path>>(&self, path: P) -> Result<(), PersistError> {
        let file = File::create(path)?;
        self.client.save_world(file)
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

    // Plugins

    pub fn load_plugins(&mut self) -> QStringList {
        let Err(errors) = self.client.load_plugins() else {
            return QStringList::default();
        };
        let mut list: QStringList = QStringList::default();
        list.reserve(2 * errors.len().cast_signed());
        for error in &errors {
            list.append(QString::from(&*error.path.to_string_lossy()));
            list.append(QString::from(&error.error.to_string()));
        }
        list
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        self.timers.borrow_mut().reset_plugin(index);
        self.client.reinstall_plugin(index)
    }

    pub fn reset_plugins(&self) -> Vec<ffi::PluginPack> {
        self.client.reset_plugins();
        self.timers.borrow_mut().clear();
        self.client.plugins().map(ffi::PluginPack::from).collect()
    }

    pub fn reset_world_plugin(&self) {
        self.timers
            .borrow_mut()
            .reset_plugin(self.world_plugin_index());
        self.client.reset_world_plugin();
    }

    /// # Panics
    ///
    /// Panics if there is no world plugin.
    #[track_caller]
    pub fn world_plugin_index(&self) -> PluginIndex {
        self.client
            .plugins()
            .position(|plugin| plugin.metadata.is_world_plugin)
            .expect("world plugin is missing!")
    }

    // Document

    fn handler<'a>(&'a self, doc: Pin<&'a mut ffi::Document>) -> ClientHandler<'a> {
        ClientHandler::new(
            doc,
            &self.formatter,
            &self.stats,
            &self.client.borrow_world(),
        )
    }

    pub fn alias(
        &self,
        command: &str,
        source: CommandSource,
        doc: Pin<&mut ffi::Document>,
    ) -> AliasOutcome {
        self.client.alias(command, source, &mut self.handler(doc))
    }

    pub fn flush(&self, doc: Pin<&mut ffi::Document>) {
        let mut handler = self.handler(doc);
        if self.client.flush_output(&mut handler) {
            handler.set_had_output();
        }
    }

    pub fn invoke_alias(&self, index: PluginIndex, id: u16, doc: Pin<&mut ffi::Document>) -> bool {
        self.client.invoke_alias(index, id, &mut self.handler(doc))
    }

    pub fn read(&self, socket: Pin<&mut QAbstractSocket>, doc: Pin<&mut ffi::Document>) -> i64 {
        let io_result = self.handle_socket_read(socket);
        let mut handler = self.handler(doc);
        if self.client.drain_output(&mut handler) {
            handler.set_had_output();
        }
        #[allow(clippy::cast_possible_truncation, clippy::cast_possible_wrap)]
        match io_result {
            Ok(total_read) => total_read as i64,
            Err(e) => {
                handler.display_error(&e.to_string());
                -1
            }
        }
    }

    pub fn simulate(&self, line: &str, doc: Pin<&mut ffi::Document>) {
        self.client.simulate_output(line, &mut self.handler(doc));
    }

    // Network

    pub fn connect_to_host(&self, mut socket: Pin<&mut QAbstractSocket>) {
        let world = self.client.borrow_world();
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
        match socket.write_all(self.client.borrow_world().connect_message().as_bytes()) {
            Ok(()) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        }
    }

    pub fn handle_disconnect(&self) {
        self.stats.borrow_mut().clear();
        self.client.reset_connection();
    }

    fn handle_socket_read(&self, mut socket: Pin<&mut QAbstractSocket>) -> io::Result<usize> {
        let n = self
            .client
            .read(&mut socket, &mut self.read_buf.borrow_mut())?;
        self.client.write(&mut socket)?;
        Ok(n)
    }

    // Sender info

    pub fn alias_info(&self, index: PluginIndex, label: &str, info_type: i64) -> QVariant {
        self.client
            .alias_info::<InfoVisitorQVariant>(index, label, info_type)
    }

    pub fn timer_info(&self, index: PluginIndex, label: &str, info_type: i64) -> QVariant {
        self.client.timer_info::<InfoVisitorQVariant, _>(
            index,
            label,
            info_type,
            &self.timers.borrow(),
        )
    }

    pub fn trigger_info(&self, index: PluginIndex, label: &str, info_type: i64) -> QVariant {
        self.client
            .trigger_info::<InfoVisitorQVariant>(index, label, info_type)
    }

    // Timers

    pub fn add_timer(
        &self,
        index: PluginIndex,
        timer: Timer,
    ) -> Result<Option<TimerStart>, SenderAccessError> {
        let timer = self.client.add_sender(index, timer)?;
        if !self.client.borrow_world().enable_timers {
            return Ok(None);
        }
        Ok(self.timers.borrow_mut().start(index, &timer))
    }

    pub fn add_or_replace_timer(&self, index: PluginIndex, timer: Timer) -> Option<TimerStart> {
        let timer = self.client.add_or_replace_sender(index, timer);
        if !self.client.borrow_world().enable_timers {
            return None;
        }
        self.timers.borrow_mut().start(index, &timer)
    }

    pub fn finish_timer(&self, id: usize) -> TimerFinish<ffi::SendTimer> {
        self.timers.borrow_mut().finish(id, &self.client)
    }

    pub fn poll_timers(&self) -> Vec<Rc<ffi::SendTimer>> {
        self.timers.borrow_mut().poll(&self.client)
    }

    pub fn import_world_timers(&self, xml: &str) -> Result<Vec<TimerStart>, ImportError> {
        let world_index = self.world_plugin_index();
        let imported_timers = self.client.import_world_senders::<Timer>(xml)?;
        if !self.client.borrow_world().enable_timers {
            return Ok(Vec::new());
        }
        let mut timers = self.timers.borrow_mut();
        Ok(imported_timers
            .iter()
            .filter_map(|timer| timers.start(world_index, timer))
            .collect())
    }

    pub fn replace_world_timer(
        &self,
        index: usize,
        timer: Timer,
    ) -> (Result<usize, ffi::ReplaceSenderResult>, Option<TimerStart>) {
        let group = timer.group.clone();
        let (i, timer) = match self.client.replace_world_sender(index, timer) {
            Ok(result) => result,
            Err(e) => return (Err(e.into()), None),
        };
        let result = if timer.group == group {
            Ok(i)
        } else {
            Err(ffi::ReplaceSenderResult::GroupChanged)
        };
        if !self.client.borrow_world().enable_timers {
            return (result, None);
        }
        let world_index = self.world_plugin_index();
        (result, self.timers.borrow_mut().start(world_index, &timer))
    }

    pub fn start_all_timers(&self) -> Vec<TimerStart> {
        if !self.client.borrow_world().enable_timers {
            return Vec::new();
        }
        let mut timer_starts = Vec::new();
        let mut timers = self.timers.borrow_mut();
        for (index, plugin_timers) in self.client.all_senders::<Timer>().enumerate() {
            timer_starts.extend(
                plugin_timers
                    .borrow()
                    .iter()
                    .filter_map(|timer| timers.start(index, timer)),
            );
        }
        timer_starts
    }

    pub fn start_timers(&self, index: PluginIndex) -> Vec<TimerStart> {
        if !self.client.borrow_world().enable_timers {
            return Vec::new();
        }
        let mut timers = self.timers.borrow_mut();
        self.client
            .senders::<Timer>(index)
            .borrow()
            .iter()
            .filter_map(|timer| timers.start(index, timer))
            .collect()
    }

    // Color

    pub fn ansi_note(&self, text: &str) -> Vec<ffi::StyledSpan> {
        mud_transformer::output::interpret_ansi(text)
            .map(|fragment| ffi::StyledSpan {
                format: self.formatter.text_format(&fragment).into_owned(),
                text: QString::from(&*fragment.text),
            })
            .collect()
    }
}
