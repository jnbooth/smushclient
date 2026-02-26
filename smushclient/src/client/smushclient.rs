use std::borrow::Cow;
use std::cell::{Ref, RefCell, RefMut};
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, BufReader, Cursor, Read, Write};
use std::path::Path;
use std::{env, mem, slice};

use arboard::Clipboard;
use flagset::FlagSet;
use mud_transformer::{
    Output, OutputFragment, Tag, TelnetFragment, Transformer, TransformerConfig,
};
use rodio::Decoder;
use smushclient_plugins::{
    Alias, CursorVec, ImportError, LoadError, Plugin, PluginIndex, PluginSender, SendTarget,
    SenderAccessError, SortOnDrop, Timer, Trigger, XmlSerError,
};
#[cfg(feature = "async")]
use tokio::io::{AsyncRead, AsyncReadExt};

use super::info::ClientInfo;
use super::logger::Logger;
use super::variables::PluginVariables;
use super::variables::{LuaStr, LuaString};
use crate::audio::{AudioError, AudioSinks, PlayMode};
use crate::get_info::InfoVisitor;
use crate::handler::Handler;
use crate::import::ImportedWorld;
use crate::options::OptionValue;
use crate::plugins::{
    AliasEffects, AliasOutcome, CommandSource, LoadFailure, PluginEngine, PluginReaction,
    SendRequest, SendScriptRequest, SpanStyle, TriggerEffects,
};
use crate::world::{LogMode, OptionCaller, PersistError, SetOptionError, World, WorldConfig};

const METAVARIABLES_KEY: &str = "\x01";

pub struct SmushClient {
    pub(crate) plugins: PluginEngine,
    logger: RefCell<Logger>,
    supported_tags: FlagSet<Tag>,
    transformer: RefCell<Transformer>,
    variables: RefCell<PluginVariables>,
    world: RefCell<WorldConfig>,
    audio: AudioSinks,
    buffers: RefCell<(String, String, HashSet<u16>)>,
    info: ClientInfo,
}

impl Default for SmushClient {
    fn default() -> Self {
        Self::new(World::default(), FlagSet::empty())
    }
}

impl SmushClient {
    /// # Panics
    ///
    /// Panics if audio initialization fails.
    pub fn new(world: World<'static>, supported_tags: FlagSet<Tag>) -> Self {
        let mut plugins = PluginEngine::new();
        let config = world.config.into_owned();
        let world_plugin = Plugin {
            aliases: world.aliases.into_owned().into(),
            timers: world.timers.into_owned().into(),
            triggers: world.triggers.into_owned().into(),
            ..config.world_plugin()
        };
        plugins.set_world_plugin(world_plugin);
        let transformer = Transformer::new(TransformerConfig {
            supports: supported_tags,
            ..TransformerConfig::from(&config)
        });
        Self {
            logger: RefCell::new(Logger::new(&config)),
            plugins,
            supported_tags,
            transformer: RefCell::new(transformer),
            variables: RefCell::default(),
            world: RefCell::new(config),
            audio: AudioSinks::try_default().expect("audio initialization error"),
            buffers: RefCell::default(),
            info: ClientInfo::default(),
        }
    }

    pub fn import_world<R: Read>(
        reader: R,
        supported_tags: FlagSet<Tag>,
    ) -> Result<Self, ImportError> {
        let reader = BufReader::new(reader);
        let ImportedWorld { world, variables } = ImportedWorld::from_xml(reader)?;
        Ok(Self {
            variables: RefCell::new(variables),
            ..Self::new(world, supported_tags)
        })
    }

    pub fn reset_world_plugin(&self) {
        self.world_plugin().remove_temporary();
    }

    pub fn reset_plugins(&self) {
        for plugin in &self.plugins {
            plugin.remove_temporary();
        }
    }

    pub fn reset_connection(&mut self) {
        *self.transformer.borrow_mut() = Transformer::new(self.create_config());
        self.info.reset();
    }

    pub fn set_supported_tags(&mut self, supported_tags: FlagSet<Tag>) {
        self.supported_tags = supported_tags;
    }

    pub fn stop_evaluating<T: PluginSender>(&self) {
        for senders in self.plugins.all_senders::<T>() {
            senders.end();
        }
    }

    fn update_logger(&self) {
        self.logger.borrow_mut().apply_world(&self.world.borrow());
    }

    fn update_config(&mut self) {
        self.transformer
            .borrow_mut()
            .set_config(self.create_config());
    }

    fn create_config(&self) -> TransformerConfig {
        TransformerConfig {
            will: self.plugins.supported_protocols(),
            supports: self.supported_tags,
            ..TransformerConfig::from(&*self.world.borrow())
        }
    }

    pub fn borrow_world(&self) -> Ref<'_, WorldConfig> {
        self.world.borrow()
    }

    /// # Panics
    ///
    /// Panics if there is no world plugin.
    pub fn world_plugin(&self) -> &Plugin {
        self.plugins
            .world_plugin()
            .expect("world plugin uninitialized")
    }

    pub fn save_world<W: Write>(&self, writer: W) -> Result<(), PersistError> {
        let world_plugin = self.world_plugin();
        let config = self.world.borrow();
        let aliases = world_plugin.aliases.borrow();
        let timers = world_plugin.timers.borrow();
        let triggers = world_plugin.triggers.borrow();
        World {
            config: Cow::Borrowed(&config),
            aliases: Cow::Borrowed(&aliases),
            timers: Cow::Borrowed(&timers),
            triggers: Cow::Borrowed(&triggers),
        }
        .save(writer)
    }

    pub fn update_world(&mut self, mut world: WorldConfig) -> bool {
        {
            let mut world_mut = self.world.borrow_mut();
            let plugins = mem::take(&mut world_mut.plugins);
            if *world_mut == world {
                world_mut.plugins = plugins;
                return false;
            }
            world.plugins = plugins;
            *world_mut = world;
            self.plugins.set_world_plugin(world_mut.world_plugin());
        }
        self.update_config();
        self.update_logger();
        true
    }

    fn option_caller(&self, index: PluginIndex) -> OptionCaller {
        if self.plugins[index].metadata.is_world_plugin {
            OptionCaller::WorldScript
        } else {
            OptionCaller::Plugin
        }
    }

    pub fn open_log(&self, mut path: String, mode: Option<LogMode>) -> io::Result<()> {
        if path.is_empty() {
            path = self.world.borrow().log_path();
        }
        let mode = mode.unwrap_or_else(|| self.world.borrow().log_mode);
        self.logger.borrow_mut().open(path, mode)
    }

    pub fn close_log(&self) -> io::Result<()> {
        self.logger.borrow_mut().close()
    }

    pub fn flush_log(&self) -> io::Result<()> {
        self.logger.borrow_mut().flush()
    }

    pub fn is_log_open(&self) -> bool {
        self.logger.borrow().is_open()
    }

    pub fn read<R: Read>(&mut self, mut reader: R, read_buf: &mut [u8]) -> io::Result<usize> {
        self.info.packets_received.update(|n| n + 1);
        let mut transformer = self.transformer.borrow_mut();
        let mut logger = self.logger.borrow_mut();
        let midpoint = read_buf.len() / 2;
        let mut total_read = 0;
        loop {
            let n = reader.read(&mut read_buf[..midpoint])?;
            if n == 0 {
                return Ok(total_read);
            }
            let (received, buf) = read_buf.split_at_mut(n);
            let _ = logger.log_raw(received);
            transformer.receive(received, buf)?;
            total_read += n;
        }
    }

    #[cfg(feature = "async")]
    pub async fn read_async<R: AsyncRead + Unpin>(
        &mut self,
        reader: &mut R,
        read_buf: &mut [u8],
    ) -> io::Result<usize> {
        self.info.packets_received.update(|n| n + 1);
        let midpoint = read_buf.len() / 2;
        let mut total_read = 0;
        loop {
            let n = reader.read(&mut read_buf[..midpoint]).await?;
            if n == 0 {
                return Ok(total_read);
            }
            let (received, buf) = read_buf.split_at_mut(n);
            let _ = self.logger.borrow_mut().log_raw(buf);
            self.transformer.borrow_mut().receive(received, buf)?;
            total_read += n;
        }
    }

    pub fn write<W: Write>(&mut self, writer: &mut W) -> io::Result<()> {
        let mut transformer = self.transformer.borrow_mut();
        let Some(mut drain) = transformer.drain_input() else {
            return Ok(());
        };
        drain.write_all_to(writer)
    }

    pub fn has_output(&self) -> bool {
        self.transformer.borrow().has_output()
    }

    pub fn drain_output<H: Handler>(&mut self, handler: &mut H) -> bool {
        self.process_output(handler, false)
    }

    pub fn flush_output<H: Handler>(&mut self, handler: &mut H) -> bool {
        self.process_output(handler, true)
    }

    fn process_output<H: Handler>(&self, handler: &mut H, flush: bool) -> bool {
        let mut transformer = self.transformer.borrow_mut();
        let drain = if flush {
            transformer.flush_output()
        } else {
            transformer.drain_output()
        };
        let mut had_output = false;
        let mut slice = drain.as_slice();
        let mut line_text = String::new();
        let mut lines_received = 0;
        for output in slice {
            match &output.fragment {
                OutputFragment::LineBreak | OutputFragment::PageBreak => lines_received += 1,
                OutputFragment::Hr => lines_received += 2,
                _ => (),
            }
        }
        if lines_received != 0 {
            self.info.lines_received.update(|n| n + lines_received);
        }
        while !slice.is_empty() {
            line_text.clear();
            let mut until = 0;
            let mut last_subnegotiation = None;
            for (i, output) in slice.iter().enumerate() {
                match &output.fragment {
                    OutputFragment::Text(fragment) => line_text.push_str(&fragment.text),
                    OutputFragment::Hr => {
                        self.info.lines_displayed.update(|n| n + 1);
                        until = i + 1;
                        break;
                    }
                    OutputFragment::LineBreak | OutputFragment::PageBreak => {
                        until = i + 1;
                        break;
                    }
                    OutputFragment::Telnet(TelnetFragment::Subnegotiation { data, .. }) => {
                        last_subnegotiation = Some(data);
                    }
                    OutputFragment::Telnet(TelnetFragment::GoAhead) => {
                        self.info
                            .last_line_with_iac_ga
                            .set(self.info.lines_displayed.get());
                    }
                    _ => (),
                }
            }
            if let Some(last_subnegotation) = last_subnegotiation {
                *self.info.last_subnegotiation.borrow_mut() = last_subnegotation.clone();
            }
            let has_break = until != 0;
            if !has_break {
                until = slice.len();
            }
            let output = &slice[..until];
            slice = &slice[until..];

            if !has_break && line_text.is_empty() {
                for fragment in output {
                    handler.display(fragment);
                }
                continue;
            }

            if !handler.permit_line(&line_text) {
                for fragment in output {
                    if !matches!(fragment.fragment, OutputFragment::Text(_)) {
                        handler.display(fragment);
                    }
                }
                continue;
            }

            for fragment in output {
                handler.display(fragment);
            }

            if has_break {
                self.info.lines_displayed.update(|n| n + 1);
            }

            let trigger_effects = self.trigger(&line_text, output, handler);

            had_output = had_output || !trigger_effects.omit_from_output;

            if !trigger_effects.omit_from_log
                && let Err(e) = self.logger.borrow_mut().log_output_line(&line_text, output)
            {
                handler.display_error(&format!("Log error: {e}"));
            }
        }

        had_output
    }

    pub fn log_note(&self, note: &str) -> io::Result<()> {
        self.logger.borrow_mut().log_note(note)
    }

    pub fn log_input(&self, input: &str) -> io::Result<()> {
        self.logger.borrow_mut().log_input_line(input)
    }

    pub fn write_to_log(&self, bytes: &[u8]) -> io::Result<()> {
        self.logger.borrow_mut().write_all(bytes)
    }

    pub fn load_plugins(&mut self) -> Result<(), Vec<LoadFailure>> {
        self.plugins.load_plugins(&self.world.borrow())?;
        self.update_config();
        Ok(())
    }

    pub fn add_plugin<P: AsRef<Path>>(
        &mut self,
        path: P,
    ) -> Result<(PluginIndex, &Plugin), LoadError> {
        let path = path.as_ref();
        let path = env::current_dir()
            .ok()
            .and_then(|cwd| path.strip_prefix(cwd).ok())
            .unwrap_or(path);
        let index = self.plugins.add_plugin(path)?.0;
        self.update_world_plugins();
        Ok((index, &self.plugins[index]))
    }

    pub fn remove_plugin(&mut self, index: PluginIndex) -> Option<Plugin> {
        let plugin = self.plugins.remove_plugin(index)?;
        let plugin_path = plugin.metadata.path.as_path();
        self.world
            .borrow_mut()
            .plugins
            .retain(|path| path != plugin_path);
        Some(plugin)
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        self.plugins.reinstall_plugin(index)
    }

    pub fn plugins(&self) -> slice::Iter<'_, Plugin> {
        self.plugins.iter()
    }

    /// # Panics
    ///
    /// Panics if the index is out of range.
    #[inline]
    #[track_caller]
    pub fn plugin(&self, index: PluginIndex) -> &Plugin {
        &self.plugins[index]
    }

    pub fn plugins_len(&self) -> usize {
        self.plugins.len()
    }

    pub fn configure_audio_sink(
        &self,
        i: usize,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        self.audio.configure_sink(i, volume, mode)
    }

    #[inline(always)] // avoid move operation on buffer
    pub fn play_buffer(
        &self,
        i: usize,
        buffer: Vec<u8>,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        let decoder = Decoder::try_from(Cursor::new(buffer))?;
        self.audio.play(i, decoder, volume, mode)
    }

    pub fn play_file<P: AsRef<Path>>(
        &self,
        i: usize,
        path: P,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        let file = File::open(path)?;
        let decoder = Decoder::try_from(file)?;
        self.audio.play(i, decoder, volume, mode)
    }

    pub fn play_file_raw<P: AsRef<Path>>(&self, path: P) -> Result<(), AudioError> {
        let file = File::open(path)?;
        let decoder = Decoder::try_from(file)?;
        self.audio.mixer().add(decoder);
        Ok(())
    }

    pub fn stop_sound(&self, i: usize) -> Result<(), AudioError> {
        self.audio.stop(i)
    }

    pub fn has_variables(&self) -> bool {
        !self.variables.borrow().is_empty()
    }

    pub fn variables_len(&self, index: PluginIndex) -> usize {
        let plugin_id = &self.plugins[index].metadata.id;
        self.variables.borrow().count_variables(plugin_id)
    }

    pub fn load_variables<R: Read>(&self, reader: R) -> Result<(), PersistError> {
        *self.variables.borrow_mut() = PluginVariables::load(reader)?;
        self.info.variables_dirty.set(false);
        Ok(())
    }

    pub fn save_variables<W: Write>(&self, writer: W) -> Result<(), PersistError> {
        self.variables.borrow().save(writer)?;
        self.info.variables_dirty.set(false);
        Ok(())
    }

    pub fn borrow_variable(&self, index: PluginIndex, key: &LuaStr) -> Option<Ref<'_, LuaStr>> {
        let plugin_id = &self.plugins[index].metadata.id;
        Ref::filter_map(self.variables.borrow(), |vars| {
            vars.get_variable(plugin_id, key)
        })
        .ok()
    }

    pub fn borrow_metavariable(&self, key: &LuaStr) -> Option<Ref<'_, LuaStr>> {
        Ref::filter_map(self.variables.borrow(), |vars| {
            vars.get_variable(METAVARIABLES_KEY, key)
        })
        .ok()
    }

    pub fn has_metavariable(&self, key: &LuaStr) -> bool {
        self.variables.borrow().has_variable(METAVARIABLES_KEY, key)
    }

    pub fn set_variable(&self, index: PluginIndex, key: LuaString, value: LuaString) -> bool {
        let plugin_id = &self.plugins[index].metadata.id;
        self.info.variables_dirty.set(true);
        self.variables
            .borrow_mut()
            .set_variable(plugin_id, key, value);
        true
    }

    pub fn unset_variable(&self, index: PluginIndex, key: &LuaStr) -> Option<LuaString> {
        let plugin_id = &self.plugins[index].metadata.id;
        self.info.variables_dirty.set(true);
        self.variables.borrow_mut().unset_variable(plugin_id, key)
    }

    pub fn set_metavariable(&self, key: LuaString, value: LuaString) -> bool {
        self.info.variables_dirty.set(true);
        self.variables
            .borrow_mut()
            .set_variable(METAVARIABLES_KEY, key, value);
        true
    }

    pub fn unset_metavariable(&self, key: &LuaStr) -> Option<LuaString> {
        self.info.variables_dirty.set(true);
        self.variables
            .borrow_mut()
            .unset_variable(METAVARIABLES_KEY, key)
    }

    pub fn set_group_enabled<T: PluginSender>(
        &self,
        index: PluginIndex,
        group: &str,
        enabled: bool,
    ) -> usize {
        let mut count = 0;
        for sender in self
            .senders::<T>(index)
            .borrow_mut()
            .iter_mut()
            .filter(|sender| sender.as_ref().group == group)
        {
            sender.as_mut().enabled = enabled;
            count += 1;
        }
        count
    }

    pub fn set_plugin_enabled(&self, index: PluginIndex, enabled: bool) {
        self.plugins[index].disabled.set(!enabled);
    }

    pub fn borrow_sender<T: PluginSender>(
        &self,
        index: PluginIndex,
        label: &str,
    ) -> Option<Ref<'_, T>> {
        self.senders::<T>(index)
            .find(|sender| sender.as_ref().label == label)
    }

    pub fn borrow_sender_mut<T: PluginSender>(
        &self,
        index: PluginIndex,
        label: &str,
    ) -> Result<RefMut<'_, T>, SenderAccessError> {
        self.senders::<T>(index)
            .find_mut(|sender| sender.as_ref().label == label)
            .ok_or(SenderAccessError::NotFound)
    }

    pub fn set_sender_enabled<T: PluginSender>(
        &self,
        index: PluginIndex,
        label: &str,
        enabled: bool,
    ) -> Result<(), SenderAccessError> {
        self.borrow_sender_mut::<T>(index, label)?.as_mut().enabled = enabled;
        Ok(())
    }

    pub fn senders<T: PluginSender>(&self, index: PluginIndex) -> &CursorVec<T> {
        self.plugins[index].senders::<T>()
    }

    pub fn world_senders<T: PluginSender>(&self) -> &CursorVec<T> {
        self.world_plugin().senders::<T>()
    }

    pub fn add_sender<T: PluginSender>(
        &self,
        index: PluginIndex,
        mut sender: T,
    ) -> Result<Ref<'_, T>, SenderAccessError> {
        let plugin = &self.plugins[index];
        let senders = plugin.senders::<T>();
        sender.assert_unique_label(senders)?;
        if plugin.metadata.is_world_plugin {
            sender.as_mut().temporary = true;
        }
        Ok(senders.insert(sender))
    }

    pub fn remove_sender<T: PluginSender>(
        &self,
        index: PluginIndex,
        label: &str,
    ) -> Result<(), SenderAccessError> {
        let senders = self.senders::<T>(index);
        let pos = senders
            .position(|sender| sender.as_ref().label == label)
            .ok_or(SenderAccessError::NotFound)?;
        if !senders.remove(pos) {
            return Err(SenderAccessError::ItemInUse);
        }
        Ok(())
    }

    pub fn remove_sender_group<T: PluginSender>(&self, index: PluginIndex, group: &str) -> usize {
        self.senders::<T>(index)
            .retain(|sender: &T| sender.as_ref().group != group)
    }

    pub fn remove_temporary_senders<T: PluginSender>(&self) -> usize {
        self.plugins
            .all_senders::<T>()
            .map(|senders| senders.retain(|sender: &T| !sender.as_ref().temporary))
            .sum()
    }

    pub fn add_or_replace_sender<T: PluginSender>(
        &self,
        index: PluginIndex,
        sender: T,
    ) -> Ref<'_, T> {
        let senders = self.senders::<T>(index);
        match sender.assert_unique_label(senders) {
            Ok(()) => senders.insert(sender),
            Err(replace_at) => senders.replace(replace_at, sender).1,
        }
    }

    pub fn add_world_sender<T: PluginSender>(
        &self,
        sender: T,
    ) -> Result<Ref<'_, T>, SenderAccessError> {
        Ok(self.world_plugin().add_sender(sender)?)
    }

    pub fn replace_world_sender<T: PluginSender>(
        &self,
        index: usize,
        sender: T,
    ) -> Result<(usize, Ref<'_, T>), SenderAccessError> {
        self.world_plugin().replace_sender(index, sender)
    }

    pub fn export_world_senders<T: PluginSender>(&self) -> Result<String, XmlSerError> {
        T::list_to_xml_string(
            self.world_plugin()
                .senders::<T>()
                .borrow()
                .iter()
                .filter(|sender| !sender.as_ref().temporary),
        )
    }

    pub fn import_world_senders<T: PluginSender>(
        &self,
        xml: &str,
    ) -> Result<SortOnDrop<'_, T>, ImportError> {
        let mut senders = T::list_from_xml_str(xml)?;
        Ok(self.world_plugin().import_senders(&mut senders))
    }

    pub fn export_sender<T: PluginSender>(
        &self,
        index: PluginIndex,
        name: &str,
    ) -> Result<String, XmlSerError> {
        let Some(sender) = self.borrow_sender::<T>(index, name) else {
            return Ok(String::new());
        };
        sender.to_xml_string()
    }

    pub fn export_numpad_key(&self, name: &str) -> Result<String, XmlSerError> {
        self.world.borrow().numpad_shortcuts.export_key(name)
    }

    pub fn export_variable(&self, index: PluginIndex, name: &str) -> Result<String, XmlSerError> {
        let plugin_id = &self.plugins[index].metadata.id;
        self.variables.borrow().export_variable(plugin_id, name)
    }

    pub fn world_option(&self, index: PluginIndex, option: &LuaStr) -> Option<i64> {
        let caller = self.option_caller(index);
        self.world.borrow().option_int(caller, option)
    }

    pub fn world_alpha_option(
        &self,
        index: PluginIndex,
        option: &LuaStr,
    ) -> Option<Ref<'_, LuaStr>> {
        let caller = self.option_caller(index);
        Ref::filter_map(self.world.borrow(), |world: &WorldConfig| {
            world.option_str(caller, option)
        })
        .ok()
    }

    pub fn world_variant_option(&self, index: PluginIndex, option: &LuaStr) -> OptionValue<'_> {
        let caller = self.option_caller(index);
        if let Some(value) = self.world.borrow().option_int(caller, option) {
            OptionValue::Numeric(value)
        } else if let Ok(value) = Ref::filter_map(self.world.borrow(), |world: &WorldConfig| {
            world.option_str(caller, option)
        }) {
            OptionValue::Borrow(value)
        } else {
            OptionValue::Null
        }
    }

    pub fn set_world_option(
        &mut self,
        index: PluginIndex,
        option: &LuaStr,
        value: i64,
    ) -> Result<(), SetOptionError> {
        let caller = self.option_caller(index);
        self.world
            .borrow_mut()
            .set_option_int(caller, option, value)?;
        match option {
            b"connect_method"
            | b"convert_ga_to_newline"
            | b"disable_compression"
            | b"disable_utf8"
            | b"ignore_mxp_colors"
            | b"naws"
            | b"no_echo_off"
            | b"use_mxp" => self.update_config(),
            b"echo_colour" | b"log_format" | b"log_in_colour" | b"log_mode" | b"note_colour" => {
                self.update_logger();
            }
            _ => (),
        }
        Ok(())
    }

    pub fn set_world_alpha_option(
        &mut self,
        index: PluginIndex,
        option: &LuaStr,
        value: LuaString,
    ) -> Result<(), SetOptionError> {
        let caller = self.option_caller(index);
        self.world
            .borrow_mut()
            .set_option_str(caller, option, value)?;
        if option.starts_with(b"log_") {
            self.update_logger();
            return Ok(());
        }
        match option {
            b"name" | b"player" => {
                self.update_config();
                self.update_logger();
            }
            b"password" | b"terminal_identification" => self.update_config(),
            _ => (),
        }
        Ok(())
    }

    pub fn build_alias_menu<F>(&self, mut f: F)
    where
        F: FnMut(PluginIndex, u16, &str),
    {
        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled.get() {
                continue;
            }
            for alias in plugin.aliases.borrow().iter() {
                if alias.menu && !alias.label.is_empty() {
                    f(plugin_index, alias.id, &alias.label);
                }
            }
        }
    }

    pub fn alias<H: Handler>(
        &self,
        input: &str,
        source: CommandSource,
        handler: &mut H,
    ) -> AliasOutcome {
        let mut effects = AliasEffects::new(&self.world.borrow(), source);
        self.process_matches::<Alias, _>(input, &[], handler, &mut effects);
        effects.into()
    }

    pub fn invoke_alias<H>(&self, plugin_index: PluginIndex, id: u16, handler: &mut H) -> bool
    where
        H: Handler,
    {
        let enable_scripts = self.world.borrow().enable_scripts;
        let plugin = &self.plugins[plugin_index];
        let aliases = &plugin.aliases;
        let mut buffers = self.buffers.borrow_mut();
        let (text_buf, destination_buf, _) = &mut *buffers;
        let (send_request, has_script) = {
            let alias = match aliases.find(|alias| alias.id == id) {
                Some(alias) if alias.enabled => alias,
                _ => return false,
            };
            let enable_scripts = !plugin.metadata.is_world_plugin || enable_scripts;
            let text = alias.expand_text_captureless().into_owned();
            if alias.send_to == SendTarget::Variable {
                self.variables.borrow_mut().set_variable(
                    &plugin.metadata.id,
                    alias.variable.as_bytes().to_vec(),
                    text.into_bytes(),
                );
                (None, !alias.script.is_empty())
            } else if !enable_scripts && alias.send_to.is_script() {
                (None, !alias.script.is_empty())
            } else {
                *text_buf = text;
                destination_buf.clone_from(alias.destination());
                (
                    Some(SendRequest {
                        plugin: plugin_index,
                        send_to: alias.send_to,
                        echo: !alias.omit_from_output,
                        log: !alias.omit_from_log,
                        text: text_buf,
                        destination: destination_buf,
                    }),
                    !alias.script.is_empty(),
                )
            }
        };
        if let Some(send_request) = send_request {
            handler.send(send_request);
        }
        if !has_script {
            return true;
        }
        let regex = {
            // fresh borrow in case the handler changed the reaction's settings
            let alias = match aliases.find(|alias| alias.id == id) {
                Some(alias) if alias.enabled => alias,
                _ => return true,
            };
            if alias.script.is_empty() {
                return true;
            }
            destination_buf.clone_from(&alias.script);
            text_buf.clone_from(&alias.label);
            alias.regex.clone()
        };
        handler.send_script(SendScriptRequest {
            plugin: plugin_index,
            script: destination_buf,
            label: text_buf,
            line: "",
            regex: &regex,
            wildcards: None,
            output: &[],
        });
        true
    }

    fn trigger<H: Handler>(
        &self,
        line: &str,
        output: &[Output],
        handler: &mut H,
    ) -> TriggerEffects {
        let mut effects = TriggerEffects::new();
        self.process_matches::<Trigger, _>(line, output, handler, &mut effects);
        if effects.omit_from_output {
            handler.erase_last_line();
        }
        effects
    }

    pub fn simulate_output<H: Handler>(&self, line: &str, handler: &mut H) {
        self.info.simulating.set(true);
        self.trigger(
            line,
            &[
                OutputFragment::Text(line.into()).into(),
                OutputFragment::LineBreak.into(),
            ],
            handler,
        );
        self.info.simulating.set(false);
    }

    fn process_matches<T: PluginReaction, H: Handler>(
        &self,
        line: &str,
        output: &[Output],
        handler: &mut H,
        effects: &mut T::Effects,
    ) {
        if !T::enabled(&self.world.borrow()) {
            return;
        }
        let mut buffers = self.buffers.borrow_mut();
        let (text_buf, destination_buf, one_shots) = &mut *buffers;
        let mut can_echo = true;
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled.get() {
                continue;
            }
            let enable_scripts =
                !plugin.metadata.is_world_plugin || self.world.borrow().enable_scripts;
            let senders = plugin.senders::<T>();
            for sender in senders.scan() {
                let mut matched = false;
                let (echo_input, regex_rc) = {
                    let sender = sender.borrow();
                    let reaction = sender.reaction();
                    if !reaction.enabled {
                        continue;
                    }
                    (can_echo && sender.echo_input(), reaction.regex.clone())
                };
                let regex = &*regex_rc;
                text_buf.clear();
                for captures in regex.captures_iter(line).filter_map(Result::ok) {
                    if !matched {
                        matched = true;
                        if echo_input {
                            handler.echo(line);
                            can_echo = false;
                        }
                        if T::AFFECTS_STYLE {
                            let sender = sender.borrow();
                            style = sender.style();
                            has_style = !style.is_null();
                        }
                    }
                    if has_style && let Some(capture) = captures.get(0) {
                        handler.apply_styles(capture.start()..capture.end(), style);
                    }
                    let send_request = {
                        let sender = sender.borrow();
                        if let Some(clipboard_arg) = sender.clipboard_arg()
                            && let Some(capture) = captures.get(clipboard_arg.get().into())
                            && let Ok(mut clipboard) = Clipboard::new()
                        {
                            clipboard.set_text(capture.as_str()).ok();
                        }
                        let reaction = sender.reaction();
                        if reaction.send_to == SendTarget::Variable {
                            self.variables.borrow_mut().set_variable(
                                &plugin.metadata.id,
                                reaction.variable.as_bytes().to_vec(),
                                reaction
                                    .expand_text(text_buf, &captures)
                                    .as_bytes()
                                    .to_vec(),
                            );
                            None
                        } else if !enable_scripts && reaction.send_to.is_script() {
                            None
                        } else {
                            let text = reaction.expand_text(text_buf, &captures);
                            destination_buf.clone_from(reaction.destination());
                            Some(SendRequest {
                                plugin: plugin_index,
                                send_to: reaction.send_to,
                                echo: !reaction.omit_from_output,
                                log: !reaction.omit_from_log,
                                text,
                                destination: destination_buf,
                            })
                        }
                    };
                    if let Some(send_request) = send_request {
                        handler.send(send_request);
                    }
                    // fresh borrow in case the handler changed the reaction's settings
                    if !sender.borrow().reaction().repeats {
                        break;
                    }
                }
                if !matched {
                    continue;
                }
                let send_script = enable_scripts && {
                    let sender = sender.borrow();
                    let reaction = sender.reaction();
                    if reaction.script.is_empty() {
                        false
                    } else {
                        destination_buf.clone_from(&reaction.script);
                        text_buf.clone_from(&reaction.label);
                        true
                    }
                };
                if send_script {
                    for captures in regex.captures_iter(line).filter_map(Result::ok) {
                        handler.send_script(SendScriptRequest {
                            plugin: plugin_index,
                            script: destination_buf,
                            label: text_buf,
                            line,
                            regex,
                            wildcards: Some(captures),
                            output,
                        });
                    }
                }
                let sender = sender.borrow();
                if let Some(sound) = sender.sound()
                    && self.world.borrow().enable_trigger_sounds
                    && let Ok(file) = File::open(sound)
                    && let Ok(decoder) = Decoder::try_from(file)
                {
                    self.audio.mixer().add(decoder);
                }
                sender.add_effects(effects);
                let reaction = sender.reaction();
                if reaction.one_shot {
                    one_shots.insert(reaction.id);
                }
                if !reaction.keep_evaluating {
                    break;
                }
            }
            if !one_shots.is_empty() {
                senders.retain(|sender| !one_shots.contains(&sender.as_ref().id));
                one_shots.clear();
            }
        }
    }

    fn update_world_plugins(&mut self) {
        let mut world_mut = self.world.borrow_mut();
        world_mut.plugins.clear();
        world_mut.plugins.extend(
            self.plugins
                .iter()
                .filter(|plugin| !plugin.metadata.is_world_plugin)
                .map(|plugin| plugin.metadata.path.clone()),
        );
    }

    fn count_senders<T: PluginSender>(&self) -> usize {
        self.plugins.all_senders::<T>().map(CursorVec::len).sum()
    }

    pub fn get_info<V: InfoVisitor>(&self, info_type: i64) -> V::Output {
        let info = &self.info;
        let world = self.world.borrow();
        match info_type {
            1 => V::visit(&world.site),
            2 => V::visit(&world.name),
            3 => V::visit(&world.player),
            9 => V::visit(&world.new_activity_sound),
            11 => V::visit(&world.log_file_preamble),
            12 => V::visit(&world.log_file_postamble),
            13 => V::visit(&world.log_line_preamble_input),
            14 => V::visit(&world.log_line_preamble_notes),
            15 => V::visit(&world.log_line_preamble_output),
            16 => V::visit(&world.log_line_postamble_input),
            17 => V::visit(&world.log_line_postamble_notes),
            18 => V::visit(&world.log_line_postamble_output),
            22 => V::visit(&world.connect_text),
            28 => V::visit("lua"),
            35 => V::visit(&world.world_script),
            40 => V::visit(&world.auto_log_file_name),
            42 => V::visit(&world.terminal_identification),
            51 => V::visit(self.logger.borrow().path().unwrap_or_default()),
            75 => V::visit(&**info.last_subnegotiation.borrow()),
            103 => V::visit(self.transformer.borrow().decompressing()),
            104 => V::visit(self.transformer.borrow().mxp_active()),
            105 => V::visit(false),
            118 => V::visit(info.variables_dirty.get()),
            123 => V::visit(info.simulating.get()),
            201 => V::visit(info.lines_received.get()),
            202 => V::visit(info.lines_received.get() - info.lines_displayed.get()),
            204 => V::visit(info.packets_received.get()),
            208 => V::visit(if self.transformer.borrow().decompressing() {
                2
            } else {
                0
            }),
            218 => V::visit(self.variables.borrow().len()),
            219 => V::visit(self.count_senders::<Trigger>()),
            220 => V::visit(self.count_senders::<Timer>()),
            221 => V::visit(self.count_senders::<Alias>()),
            225 => V::visit(self.transformer.borrow().count_custom_mxp_elements()),
            226 => V::visit(self.transformer.borrow().count_custom_mxp_entities()),
            231 => V::visit(match self.logger.borrow_mut().len() {
                Ok(Some(len)) => len,
                _ => 0,
            }),
            289 => V::visit(info.last_line_with_iac_ga.get()),
            _ => V::visit_none(),
        }
    }
}
