use std::cell::{Ref, RefCell, RefMut};
use std::collections::HashSet;
use std::fs::File;
use std::io::{self, Cursor, Read, Write};
use std::path::Path;
use std::{env, mem, slice};

use arboard::Clipboard;
use flagset::FlagSet;
use mud_transformer::{Output, OutputFragment, Tag, Transformer, TransformerConfig};
use rodio::Decoder;
use smushclient_plugins::{
    Alias, CursorVec, LoadError, Plugin, PluginIndex, SendTarget, Trigger, XmlError, XmlSerError,
};
#[cfg(feature = "async")]
use tokio::io::{AsyncRead, AsyncReadExt};

use super::logger::Logger;
use super::variables::PluginVariables;
use super::variables::{LuaStr, LuaString};
use crate::audio::{AudioError, AudioSinks, PlayMode};
use crate::collections::SortOnDrop;
use crate::handler::Handler;
use crate::options::OptionValue;
use crate::plugins::{
    AliasEffects, AliasOutcome, CommandSource, LoadFailure, PluginEngine, ReactionIterable,
    SendIterable, SendRequest, SendScriptRequest, SenderAccessError, SpanStyle, TriggerEffects,
};
use crate::world::{OptionCaller, PersistError, SetOptionError, World};

const METAVARIABLES_KEY: &str = "\x01";

pub struct SmushClient {
    logger: RefCell<Logger>,
    pub(crate) plugins: PluginEngine,
    supported_tags: FlagSet<Tag>,
    transformer: RefCell<Transformer>,
    variables: RefCell<PluginVariables>,
    world: World,
    audio: AudioSinks,
    buffers: RefCell<(String, String, HashSet<u16>)>,
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
    pub fn new(world: World, supported_tags: FlagSet<Tag>) -> Self {
        let mut plugins = PluginEngine::new();
        plugins.set_world_plugin(world.world_plugin());
        let transformer = Transformer::new(TransformerConfig {
            supports: supported_tags,
            ..TransformerConfig::from(&world)
        });
        Self {
            logger: RefCell::new(Logger::new(&world)),
            plugins,
            supported_tags,
            transformer: RefCell::new(transformer),
            variables: RefCell::new(PluginVariables::new()),
            world,
            audio: AudioSinks::try_default().expect("audio initialization error"),
            buffers: RefCell::new((String::new(), String::new(), HashSet::new())),
        }
    }

    pub fn reset_world_plugin(&self) {
        self.world.remove_temporary();
    }

    pub fn reset_plugins(&self) {
        for plugin in &self.plugins {
            if plugin.metadata.is_world_plugin {
                self.world.remove_temporary();
            } else {
                plugin.remove_temporary();
            }
        }
    }

    pub fn reset_connection(&mut self) {
        *self.transformer.borrow_mut() = Transformer::new(self.create_config());
    }

    pub fn set_supported_tags(&mut self, supported_tags: FlagSet<Tag>) {
        self.supported_tags = supported_tags;
    }

    pub fn stop_evaluating<T: SendIterable>(&self) {
        for plugin in &self.plugins {
            T::from_either(plugin, &self.world).end();
        }
    }

    fn update_logger(&self) -> io::Result<()> {
        self.logger.borrow_mut().apply_world(&self.world)
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
            ..TransformerConfig::from(&self.world)
        }
    }

    pub fn world(&self) -> &World {
        &self.world
    }

    pub fn set_world(&mut self, world: World) {
        self.world = world;
        self.plugins.set_world_plugin(self.world.world_plugin());
        self.update_config();
        *self.logger.borrow_mut() = Logger::new(&self.world);
    }

    pub fn update_world(&mut self, mut world: World) -> io::Result<bool> {
        if self.world == world {
            return Ok(false);
        }
        let plugins = mem::take(&mut self.world.plugins);
        let aliases = mem::take(&mut self.world.aliases);
        let timers = mem::take(&mut self.world.timers);
        let triggers = mem::take(&mut self.world.triggers);
        if self.world == world {
            self.world.plugins = plugins;
            self.world.aliases = aliases;
            self.world.timers = timers;
            self.world.triggers = triggers;
            return Ok(false);
        }
        world.plugins = plugins;
        world.aliases = aliases;
        world.timers = timers;
        world.triggers = triggers;
        self.world = world;
        self.plugins.set_world_plugin(self.world.world_plugin());
        self.update_config();
        self.update_logger()?;
        Ok(true)
    }

    fn option_caller(&self, index: PluginIndex) -> OptionCaller {
        let Some(plugin) = self.plugin(index) else {
            return OptionCaller::WorldScript;
        };
        if plugin.metadata.is_world_plugin {
            OptionCaller::WorldScript
        } else {
            OptionCaller::Plugin
        }
    }

    pub fn open_log(&self) -> io::Result<()> {
        self.logger.borrow_mut().open()
    }

    pub fn close_log(&self) -> io::Result<()> {
        self.logger.borrow_mut().close()
    }

    pub fn read<R: Read>(&mut self, mut reader: R, read_buf: &mut [u8]) -> io::Result<usize> {
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
        while !slice.is_empty() {
            line_text.clear();
            let mut until = 0;
            for (i, output) in slice.iter().enumerate() {
                match &output.fragment {
                    OutputFragment::Text(fragment) => line_text.push_str(&fragment.text),
                    OutputFragment::Hr | OutputFragment::LineBreak | OutputFragment::PageBreak => {
                        until = i + 1;
                        break;
                    }
                    _ => (),
                }
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
                    if !fragment.fragment.is_visual() {
                        handler.display(fragment);
                    }
                }
                continue;
            }

            for fragment in output {
                handler.display(fragment);
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

    fn log_input<H: Handler>(&self, input: &str, handler: &mut H) {
        if let Err(e) = self.logger.borrow_mut().log_input_line(input) {
            handler.display_error(&format!("Log error: {e}"));
        }
    }

    pub fn load_plugins(&mut self) -> Result<(), Vec<LoadFailure>> {
        self.plugins.load_plugins(&self.world)?;
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
        self.world.plugins.retain(|path| path != plugin_path);
        Some(plugin)
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        self.plugins.reinstall_plugin(index)
    }

    pub fn plugins(&self) -> slice::Iter<'_, Plugin> {
        self.plugins.iter()
    }

    pub fn plugin(&self, index: PluginIndex) -> Option<&Plugin> {
        self.plugins.get(index)
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
        self.variables
            .borrow()
            .values()
            .any(|variables| !variables.is_empty())
    }

    pub fn variables_len(&self, index: PluginIndex) -> Option<usize> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
        Some(self.variables.borrow().get(plugin_id)?.len())
    }

    pub fn load_variables<R: Read>(&self, reader: R) -> Result<(), PersistError> {
        *self.variables.borrow_mut() = PluginVariables::load(reader)?;
        Ok(())
    }

    pub fn save_variables<W: Write>(&self, writer: W) -> Result<(), PersistError> {
        self.variables.borrow().save(writer)
    }

    pub fn borrow_variable(&self, index: PluginIndex, key: &LuaStr) -> Option<Ref<'_, LuaStr>> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
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
        let Some(plugin) = self.plugins.get(index) else {
            return false;
        };
        let plugin_id = &plugin.metadata.id;
        self.variables
            .borrow_mut()
            .set_variable(plugin_id, key, value);
        true
    }

    pub fn unset_variable(&self, index: PluginIndex, key: &LuaStr) -> Option<LuaString> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
        self.variables.borrow_mut().unset_variable(plugin_id, key)
    }

    pub fn set_metavariable(&self, key: LuaString, value: LuaString) -> bool {
        self.variables
            .borrow_mut()
            .set_variable(METAVARIABLES_KEY, key, value);
        true
    }

    pub fn unset_metavariable(&self, key: &LuaStr) -> Option<LuaString> {
        self.variables
            .borrow_mut()
            .unset_variable(METAVARIABLES_KEY, key)
    }

    pub fn set_group_enabled<T: SendIterable>(
        &self,
        index: PluginIndex,
        group: &str,
        enabled: bool,
    ) -> bool {
        let mut found_group = false;
        for sender in self
            .senders::<T>(index)
            .borrow_mut()
            .iter_mut()
            .filter(|sender| sender.as_ref().group == group)
        {
            sender.as_mut().enabled = enabled;
            found_group = true;
        }
        found_group
    }

    pub fn set_plugin_enabled(&self, index: PluginIndex, enabled: bool) -> bool {
        let Some(plugin) = self.plugins.get(index) else {
            return false;
        };
        plugin.disabled.set(!enabled);
        true
    }

    pub fn borrow_sender<T: SendIterable>(
        &self,
        index: PluginIndex,
        label: &str,
    ) -> Option<Ref<'_, T>> {
        self.senders::<T>(index)
            .find(|sender| sender.as_ref().label == label)
    }

    pub fn borrow_sender_mut<T: SendIterable>(
        &self,
        index: PluginIndex,
        label: &str,
    ) -> Result<RefMut<'_, T>, SenderAccessError> {
        self.senders::<T>(index)
            .find_mut(|sender| sender.as_ref().label == label)
            .ok_or(SenderAccessError::NotFound)
    }

    pub fn set_sender_enabled<T: SendIterable>(
        &self,
        index: PluginIndex,
        label: &str,
        enabled: bool,
    ) -> Result<(), SenderAccessError> {
        self.borrow_sender_mut::<T>(index, label)?.as_mut().enabled = enabled;
        Ok(())
    }

    pub fn senders<T: SendIterable>(&self, index: PluginIndex) -> &CursorVec<T> {
        T::from_either(&self.plugins[index], &self.world)
    }

    pub fn world_senders<T: SendIterable>(&self) -> &CursorVec<T> {
        T::from_world(&self.world)
    }

    pub fn add_sender<T: SendIterable>(
        &self,
        index: PluginIndex,
        mut sender: T,
    ) -> Result<Ref<'_, T>, SenderAccessError> {
        let plugin = &self.plugins[index];
        let senders = if plugin.metadata.is_world_plugin {
            T::from_world(&self.world)
        } else {
            sender.as_mut().temporary = true;
            T::from_plugin(plugin)
        };
        sender.assert_unique_label(senders)?;
        Ok(senders.insert(sender))
    }

    pub fn remove_sender<T: SendIterable>(
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

    pub fn remove_sender_group<T: SendIterable>(&self, index: PluginIndex, group: &str) -> usize {
        self.senders::<T>(index)
            .retain(|sender: &T| sender.as_ref().group != group)
    }

    pub fn add_or_replace_sender<T: SendIterable>(
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

    pub fn add_world_sender<T: SendIterable>(
        &self,
        sender: T,
    ) -> Result<Ref<'_, T>, SenderAccessError> {
        self.world.add_sender(sender)
    }

    pub fn replace_world_sender<T: SendIterable>(
        &self,
        index: usize,
        sender: T,
    ) -> Result<(usize, Ref<'_, T>), SenderAccessError> {
        self.world.replace_sender(index, sender)
    }

    pub fn import_world_senders<T: SendIterable>(
        &self,
        xml: &str,
    ) -> Result<SortOnDrop<'_, T>, XmlError> {
        let mut senders = T::from_xml_str(xml)?;
        Ok(self.world.import_senders(&mut senders))
    }

    pub fn export_world_senders<T: SendIterable>(&self) -> Result<String, XmlSerError> {
        T::to_xml_string(&*T::from_world(&self.world).borrow())
    }

    pub fn world_option(&self, index: PluginIndex, option: &LuaStr) -> Option<i32> {
        let caller = self.option_caller(index);
        self.world.option_int(caller, option)
    }

    pub fn world_alpha_option(&self, index: PluginIndex, option: &LuaStr) -> Option<&LuaStr> {
        let caller = self.option_caller(index);
        self.world.option_str(caller, option)
    }

    pub fn world_variant_option(&self, index: PluginIndex, option: &LuaStr) -> OptionValue<'_> {
        let caller = self.option_caller(index);
        if let Some(value) = self.world.option_int(caller, option) {
            OptionValue::Numeric(value)
        } else if let Some(value) = self.world.option_str(caller, option) {
            OptionValue::Alpha(value)
        } else {
            OptionValue::Null
        }
    }

    pub fn set_world_option(
        &mut self,
        index: PluginIndex,
        option: &LuaStr,
        value: i32,
    ) -> Result<(), SetOptionError> {
        let caller = self.option_caller(index);
        self.world.set_option_int(caller, option, value)?;
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
                self.update_logger()?;
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
        self.world.set_option_str(caller, option, value)?;
        if option.starts_with(b"log_") {
            self.update_logger()?;
            return Ok(());
        }
        match option {
            b"name" | b"player" => {
                self.update_config();
                self.update_logger()?;
            }
            b"password" | b"terminal_identification" => self.update_config(),
            b"auto_log_file_name" => self.update_logger()?,
            _ => (),
        }
        Ok(())
    }

    pub fn alias<H: Handler>(
        &self,
        input: &str,
        source: CommandSource,
        handler: &mut H,
    ) -> AliasOutcome {
        let mut effects = AliasEffects::new(&self.world, source);
        let echo = match source {
            CommandSource::Hotkey => self.world.echo_hotkey_in_output_window,
            CommandSource::Link => self.world.echo_hyperlink_in_output_window,
            CommandSource::User => true,
        };
        self.process_matches::<Alias, _>(input, &[], echo, handler, &mut effects);
        if !effects.suppress {
            self.log_input(input, handler);
        }
        effects.into()
    }

    fn trigger<H: Handler>(
        &self,
        line: &str,
        output: &[Output],
        handler: &mut H,
    ) -> TriggerEffects {
        let mut effects = TriggerEffects::new();
        self.process_matches::<Trigger, _>(line, output, true, handler, &mut effects);
        if effects.omit_from_output {
            handler.erase_last_line();
        }
        effects
    }

    #[allow(clippy::too_many_arguments)]
    fn process_matches<T: ReactionIterable, H: Handler>(
        &self,
        line: &str,
        output: &[Output],
        permit_echo: bool,
        handler: &mut H,
        effects: &mut T::Effects,
    ) {
        if !T::enabled(&self.world) {
            return;
        }
        let mut buffers = self.buffers.borrow_mut();
        let (text_buf, destination_buf, one_shots) = &mut *buffers;
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled.get() {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || self.world.enable_scripts;
            let senders = if plugin.metadata.is_world_plugin {
                T::from_world(&self.world)
            } else {
                T::from_plugin(plugin)
            };
            for sender in senders.scan() {
                let mut matched = false;
                let regex_rc = {
                    let sender = sender.borrow();
                    let reaction = sender.reaction();
                    if !reaction.enabled {
                        continue;
                    }
                    reaction.regex.clone()
                };
                let regex = &*regex_rc;
                text_buf.clear();
                for captures in regex.captures_iter(line).filter_map(Result::ok) {
                    if !matched {
                        matched = true;
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
                            if !reaction.omit_from_log {
                                self.log_input(text, handler);
                            }
                            Some(SendRequest {
                                plugin: plugin_index,
                                send_to: reaction.send_to,
                                echo: permit_echo && reaction.should_echo(),
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
                    && self.world.enable_trigger_sounds
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
        self.world.plugins.clear();
        self.world.plugins.extend(
            self.plugins
                .iter()
                .filter(|plugin| !plugin.metadata.is_world_plugin)
                .map(|plugin| plugin.metadata.path.clone()),
        );
    }
}
