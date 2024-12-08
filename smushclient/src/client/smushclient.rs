use std::io::{self, Read, Write};
use std::path::Path;
use std::{env, mem, slice};

use super::logger::Logger;
use super::variables::PluginVariables;
use super::variables::{LuaStr, LuaString};
use crate::collections::SortOnDrop;
use crate::handler::Handler;
use crate::plugins::{
    AliasOutcome, CommandSource, LoadFailure, PluginEngine, SendIterable, SenderAccessError,
    TriggerEffects,
};
use crate::world::{PersistError, World};
use enumeration::EnumSet;
use mud_transformer::{OutputFragment, Tag, Transformer, TransformerConfig};
use smushclient_plugins::{CursorVec, LoadError, Plugin, PluginIndex, XmlError};
#[cfg(feature = "async")]
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

const BUF_LEN: usize = 1024 * 20;
const BUF_MIDPOINT: usize = BUF_LEN / 2;
const METAVARIABLES_KEY: &str = "\x01";

#[derive(Debug)]
pub struct SmushClient {
    last_log_file_name: Option<String>,
    line_text: String,
    logger: Logger,
    pub(crate) plugins: PluginEngine,
    read_buf: Vec<u8>,
    supported_tags: EnumSet<Tag>,
    transformer: Transformer,
    variables: PluginVariables,
    world: World,
}

impl Default for SmushClient {
    fn default() -> Self {
        Self::new(World::default(), EnumSet::new())
    }
}

impl SmushClient {
    pub fn new(world: World, supported_tags: EnumSet<Tag>) -> Self {
        let mut plugins = PluginEngine::new();
        plugins.set_world_plugin(world.world_plugin());
        let transformer = Transformer::new(TransformerConfig {
            supports: supported_tags,
            ..TransformerConfig::from(&world)
        });
        Self {
            last_log_file_name: world.auto_log_file_name.clone(),
            line_text: String::new(),
            logger: Logger::default(),
            plugins,
            read_buf: vec![0; BUF_LEN],
            supported_tags,
            transformer,
            variables: PluginVariables::new(),
            world,
        }
    }

    pub fn reset_connection(&mut self) {
        self.transformer = Transformer::new(self.create_config());
    }

    pub fn set_supported_tags(&mut self, supported_tags: EnumSet<Tag>) {
        self.supported_tags = supported_tags;
    }

    pub fn stop_evaluating<T: SendIterable>(&mut self) {
        for plugin in self.plugins.iter_mut() {
            T::from_either_mut(plugin, &mut self.world).end();
        }
    }

    fn update_config(&mut self) {
        self.transformer.set_config(self.create_config());
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
        self.logger.apply_world(&self.world);
        self.update_config();
    }

    pub fn update_world(&mut self, mut world: World) -> bool {
        let plugins = mem::take(&mut self.world.plugins);
        let aliases = mem::take(&mut self.world.aliases);
        let timers = mem::take(&mut self.world.timers);
        let triggers = mem::take(&mut self.world.triggers);
        let changed = self.world != world;
        world.plugins = plugins;
        world.aliases = aliases;
        world.timers = timers;
        world.triggers = triggers;
        self.world = world;
        self.plugins.set_world_plugin(self.world.world_plugin());
        self.logger.apply_world(&self.world);
        self.update_config();
        changed
    }

    pub fn open_log(&mut self) -> io::Result<()> {
        if self.world.auto_log_file_name != self.last_log_file_name {
            self.last_log_file_name = self.world.auto_log_file_name.clone();
        } else if self.logger.is_open() {
            return Ok(());
        }
        self.last_log_file_name = self.world.auto_log_file_name.clone();
        self.logger = Logger::open(&self.world)?;
        Ok(())
    }

    pub fn close_log(&mut self) {
        self.logger.close();
    }

    pub fn read<R: Read>(&mut self, mut reader: R) -> io::Result<usize> {
        let mut total_read = 0;
        loop {
            let n = reader.read(&mut self.read_buf[..BUF_MIDPOINT])?;
            if n == 0 {
                return Ok(total_read);
            }
            let (received, buf) = self.read_buf.split_at_mut(n);
            self.logger.log_raw(received);
            self.transformer.receive(received, buf)?;
            total_read += n;
        }
    }

    #[cfg(feature = "async")]
    pub async fn read_async<R: AsyncRead + Unpin>(&mut self, reader: &mut R) -> io::Result<usize> {
        let mut total_read = 0;
        loop {
            let n = reader.read(&mut self.read_buf[..BUF_MIDPOINT]).await?;
            if n == 0 {
                return Ok(total_read);
            }
            let (received, buf) = self.read_buf.split_at_mut(n);
            self.logger.log_raw(buf);
            self.transformer.receive(received, buf)?;
            total_read += n;
        }
    }

    pub fn write<W: Write>(&mut self, writer: &mut W) -> io::Result<()> {
        let Some(mut drain) = self.transformer.drain_input() else {
            return Ok(());
        };
        drain.write_all_to(writer)
    }

    #[cfg(feature = "async")]
    pub async fn write_async<W: AsyncWrite + Unpin>(&mut self, mut writer: W) -> io::Result<()> {
        if let Some(mut drain) = self.transformer.drain_input() {
            writer.write_all_buf(&mut drain).await
        } else {
            Ok(())
        }
    }

    pub fn has_output(&self) -> bool {
        self.transformer.has_output()
    }

    pub fn drain_output<H: Handler>(&mut self, handler: &mut H) -> bool {
        self.process_output(handler, false)
    }

    pub fn flush_output<H: Handler>(&mut self, handler: &mut H) -> bool {
        self.process_output(handler, true)
    }

    fn process_output<H: Handler>(&mut self, handler: &mut H, flush: bool) -> bool {
        self.logger.log_error(handler);
        let drain = if flush {
            self.transformer.flush_output()
        } else {
            self.transformer.drain_output()
        };
        let mut had_output = false;
        let mut slice = drain.as_slice();
        let enable_triggers = self.world.enable_triggers;
        loop {
            if slice.is_empty() {
                break;
            }
            self.line_text.clear();
            let mut until = 0;
            for (i, output) in slice.iter().enumerate() {
                match &output.fragment {
                    OutputFragment::Text(fragment) => self.line_text.push_str(&fragment.text),
                    OutputFragment::Hr | OutputFragment::LineBreak | OutputFragment::PageBreak => {
                        until = i + 1;
                        break;
                    }
                    _ => (),
                }
            }
            if until == 0 {
                until = slice.len();
            }
            let output = &slice[..until];
            slice = &slice[until..];

            if !handler.permit_line(&self.line_text) {
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

            let trigger_effects = if enable_triggers {
                self.plugins.trigger(
                    &self.line_text,
                    output,
                    &mut self.world,
                    &mut self.variables,
                    handler,
                )
            } else {
                TriggerEffects::default()
            };

            had_output =
                had_output || !self.line_text.is_empty() && !trigger_effects.omit_from_output;

            if !trigger_effects.omit_from_log {
                self.logger.log_output_line(self.line_text.as_bytes());
                self.logger.log_error(handler);
            }
        }

        had_output
    }

    pub fn alias<H: Handler>(
        &mut self,
        input: &str,
        source: CommandSource,
        handler: &mut H,
    ) -> AliasOutcome {
        if !self.world.enable_aliases {
            return AliasOutcome {
                display: true,
                remember: true,
                send: true,
            };
        }
        let outcome =
            self.plugins
                .alias(input, source, &mut self.world, &mut self.variables, handler);
        if !outcome.omit_from_log {
            self.logger.log_input_line(input.as_bytes());
            self.logger.log_error(handler);
        }
        outcome.into()
    }

    pub fn log_note(&mut self, note: &[u8]) {
        self.logger.log_note(note);
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

    pub fn plugins(&self) -> slice::Iter<Plugin> {
        self.plugins.iter()
    }

    pub fn plugin(&self, index: PluginIndex) -> Option<&Plugin> {
        self.plugins.get(index)
    }

    pub fn plugins_len(&self) -> usize {
        self.plugins.len()
    }

    pub fn has_variables(&self) -> bool {
        self.variables
            .values()
            .any(|variables| !variables.is_empty())
    }

    pub fn variables_len(&self, index: PluginIndex) -> Option<usize> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
        let variables = self.variables.get(plugin_id)?;
        Some(variables.len())
    }

    pub fn load_variables<R: Read>(&mut self, reader: R) -> Result<(), PersistError> {
        self.variables = PluginVariables::load(reader)?;
        Ok(())
    }

    pub fn save_variables<W: Write>(&self, writer: W) -> Result<(), PersistError> {
        self.variables.save(writer)
    }

    pub fn get_variable(&self, index: PluginIndex, key: &LuaStr) -> Option<&LuaStr> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
        self.variables.get_variable(plugin_id, key)
    }

    pub fn get_metavariable(&self, key: &LuaStr) -> Option<&LuaStr> {
        self.variables.get_variable(METAVARIABLES_KEY, key)
    }

    pub fn set_variable<K, V>(&mut self, index: PluginIndex, key: K, value: V) -> bool
    where
        K: Into<LuaString>,
        V: Into<LuaString>,
    {
        let Some(plugin) = self.plugins.get(index) else {
            return false;
        };
        let plugin_id = &plugin.metadata.id;
        self.variables.set_variable(plugin_id, key, value);
        true
    }

    pub fn unset_variable(&mut self, index: PluginIndex, key: &LuaStr) -> Option<LuaString> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
        self.variables.unset_variable(plugin_id, key)
    }

    pub fn set_metavariable<K, V>(&mut self, key: K, value: V) -> bool
    where
        K: Into<LuaString>,
        V: Into<LuaString>,
    {
        self.variables.set_variable(METAVARIABLES_KEY, key, value);
        true
    }

    pub fn unset_metavariable(&mut self, key: &LuaStr) -> Option<LuaString> {
        self.variables.unset_variable(METAVARIABLES_KEY, key)
    }

    pub fn set_group_enabled<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        group: &str,
        enabled: bool,
    ) -> bool {
        let mut found_group = false;
        for sender in self
            .senders_mut::<T>(index)
            .iter_mut()
            .map(AsMut::as_mut)
            .filter(|sender| sender.group == group)
        {
            sender.enabled = enabled;
            found_group = true;
        }
        found_group
    }

    pub fn set_plugin_enabled(&mut self, index: PluginIndex, enabled: bool) -> bool {
        let Some(plugin) = self.plugins.get_mut(index) else {
            return false;
        };
        plugin.disabled = !enabled;
        true
    }

    pub fn find_sender<T: SendIterable>(&self, index: PluginIndex, label: &str) -> Option<&T> {
        self.senders::<T>(index)
            .find(|sender| sender.as_ref().label == label)
    }

    pub fn find_sender_mut<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &str,
    ) -> Result<&mut T, SenderAccessError> {
        let sender = self
            .senders_mut::<T>(index)
            .iter_mut()
            .find(|sender| sender.as_ref().label == label)
            .ok_or(SenderAccessError::NotFound)?;
        Ok(sender)
    }

    pub fn set_sender_enabled<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &str,
        enabled: bool,
    ) -> Result<(), SenderAccessError> {
        self.find_sender_mut::<T>(index, label)?.as_mut().enabled = enabled;
        Ok(())
    }

    pub fn senders<T: SendIterable>(&self, index: PluginIndex) -> &CursorVec<T> {
        T::from_either(&self.plugins[index], &self.world)
    }

    pub(crate) fn senders_mut<T: SendIterable>(&mut self, index: PluginIndex) -> &mut CursorVec<T> {
        T::from_either_mut(&mut self.plugins[index], &mut self.world)
    }

    pub fn world_senders_mut<T: SendIterable>(&mut self) -> &mut CursorVec<T> {
        T::from_world_mut(&mut self.world)
    }

    pub fn add_sender<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        sender: T,
    ) -> Result<&T, SenderAccessError> {
        let senders = self.senders_mut::<T>(index);
        sender.assert_unique_label(senders)?;
        Ok(senders.insert(sender))
    }

    pub fn remove_sender<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &str,
    ) -> Result<(), SenderAccessError> {
        let senders = self.senders_mut::<T>(index);
        let pos = senders
            .position(|sender| sender.as_ref().label == label)
            .ok_or(SenderAccessError::NotFound)?;
        senders.remove(pos);
        Ok(())
    }

    pub fn remove_senders<T: SendIterable>(&mut self, index: PluginIndex, group: &str) -> usize {
        self.senders_mut::<T>(index)
            .retain(|sender: &T| sender.as_ref().group != group)
    }

    pub fn add_or_replace_sender<T: SendIterable>(&mut self, index: PluginIndex, sender: T) -> &T {
        let senders = self.senders_mut::<T>(index);
        match sender.assert_unique_label(senders) {
            Ok(()) => senders.insert(sender),
            Err(replace_at) => senders.replace(replace_at, sender).1,
        }
    }

    pub fn add_world_sender<T: SendIterable>(
        &mut self,
        sender: T,
    ) -> Result<&T, SenderAccessError> {
        self.world.add_sender(sender)
    }

    pub fn replace_world_sender<T: SendIterable>(
        &mut self,
        index: usize,
        sender: T,
    ) -> Result<(usize, &T), SenderAccessError> {
        self.world.replace_sender(index, sender)
    }

    pub fn remove_world_sender<T: SendIterable>(
        &mut self,
        index: usize,
    ) -> Result<(), SenderAccessError> {
        self.world.remove_sender::<T>(index)
    }

    pub fn import_world_senders<T: SendIterable>(
        &mut self,
        xml: &str,
    ) -> Result<SortOnDrop<T>, XmlError> {
        let mut senders = T::from_xml_str(xml)?;
        Ok(self.world.import_senders(&mut senders).into())
    }

    pub fn export_world_senders<T: SendIterable>(&self) -> Result<String, XmlError> {
        T::to_xml_string(T::from_world(&self.world))
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
