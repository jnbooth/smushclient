use std::cell::{Ref, RefCell, RefMut};
use std::io::{self, Read, Write};
use std::path::Path;
use std::{env, mem, slice};

use flagset::FlagSet;
use mud_transformer::{OutputFragment, Tag, Transformer, TransformerConfig};
use smushclient_plugins::{CursorVec, LoadError, Plugin, PluginIndex, XmlError, XmlSerError};
#[cfg(feature = "async")]
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

use super::logger::Logger;
use super::variables::PluginVariables;
use super::variables::{LuaStr, LuaString};
use crate::collections::SortOnDrop;
use crate::handler::Handler;
use crate::plugins::{
    AliasOutcome, CommandSource, LoadFailure, PluginEngine, SendIterable, SenderAccessError,
};
use crate::world::{PersistError, World};

const METAVARIABLES_KEY: &str = "\x01";

#[derive(Debug)]
pub struct SmushClient {
    last_log_file_name: Option<String>,
    logger: RefCell<Logger>,
    pub(crate) plugins: PluginEngine,
    supported_tags: FlagSet<Tag>,
    transformer: RefCell<Transformer>,
    variables: RefCell<PluginVariables>,
    world: World,
    #[cfg(feature = "async")]
    write_buf: Vec<u8>,
}

impl Default for SmushClient {
    fn default() -> Self {
        Self::new(World::default(), FlagSet::default())
    }
}

impl SmushClient {
    pub fn new(world: World, supported_tags: FlagSet<Tag>) -> Self {
        let mut plugins = PluginEngine::new();
        plugins.set_world_plugin(world.world_plugin());
        let transformer = RefCell::new(Transformer::new(TransformerConfig {
            supports: supported_tags,
            ..TransformerConfig::from(&world)
        }));
        Self {
            last_log_file_name: world.auto_log_file_name.clone(),
            logger: RefCell::new(Logger::default()),
            plugins,
            supported_tags,
            transformer,
            variables: RefCell::new(PluginVariables::new()),
            world,
            #[cfg(feature = "async")]
            write_buf: Vec::new(),
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

    pub fn reset_connection(&self) {
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

    fn update_config(&self) {
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
        self.logger.borrow_mut().apply_world(&self.world);
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
        self.logger.borrow_mut().apply_world(&self.world);
        self.update_config();
        changed
    }

    pub fn open_log(&mut self) -> io::Result<()> {
        if self.world.auto_log_file_name != self.last_log_file_name {
            self.last_log_file_name = self.world.auto_log_file_name.clone();
        } else if self.logger.borrow().is_open() {
            return Ok(());
        }
        self.last_log_file_name = self.world.auto_log_file_name.clone();
        *self.logger.borrow_mut() = Logger::open(&self.world)?;
        Ok(())
    }

    pub fn close_log(&mut self) {
        self.logger.borrow_mut().close();
    }

    pub fn read<R: Read>(&self, mut reader: R, read_buf: &mut [u8]) -> io::Result<usize> {
        let mut logger = self.logger.borrow_mut();
        let mut transformer = self.transformer.borrow_mut();
        let midpoint = read_buf.len() / 2;
        let mut total_read = 0;
        loop {
            let n = reader.read(&mut read_buf[..midpoint])?;
            if n == 0 {
                return Ok(total_read);
            }
            let (received, buf) = read_buf.split_at_mut(n);
            logger.log_raw(received);
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
            self.logger.borrow_mut().log_raw(buf);
            self.transformer.borrow_mut().receive(received, buf)?;
            total_read += n;
        }
    }

    pub fn write<W: Write>(&self, writer: &mut W) -> io::Result<()> {
        let mut transformer = self.transformer.borrow_mut();
        let Some(mut drain) = transformer.drain_input() else {
            return Ok(());
        };
        drain.write_all_to(writer)
    }

    #[cfg(feature = "async")]
    pub async fn write_async<W: AsyncWrite + Unpin>(&mut self, mut writer: W) -> io::Result<()> {
        {
            let mut transformer = self.transformer.borrow_mut();
            let Some(mut drain) = transformer.drain_input() else {
                return Ok(());
            };
            self.write_buf.clear();
            drain.write_all_to(&mut self.write_buf)?;
        }
        writer.write_all(&self.write_buf).await
    }

    pub fn has_output(&self) -> bool {
        self.transformer.borrow().has_output()
    }

    pub fn drain_output<H: Handler>(&self, handler: &mut H) -> bool {
        self.process_output(handler, false)
    }

    pub fn flush_output<H: Handler>(&self, handler: &mut H) -> bool {
        self.process_output(handler, true)
    }

    fn process_output<H: Handler>(&self, handler: &mut H, flush: bool) -> bool {
        self.logger.borrow_mut().log_error(handler);
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

            let trigger_effects =
                self.plugins
                    .trigger(&line_text, output, &self.world, &self.variables, handler);

            had_output = had_output || !trigger_effects.omit_from_output;

            if !trigger_effects.omit_from_log {
                let mut logger = self.logger.borrow_mut();
                logger.log_output_line(line_text.as_bytes());
                logger.log_error(handler);
            }
        }

        had_output
    }

    pub fn alias<H: Handler>(
        &self,
        input: &str,
        source: CommandSource,
        handler: &mut H,
    ) -> AliasOutcome {
        let outcome = self
            .plugins
            .alias(input, source, &self.world, &self.variables, handler);
        if !outcome.omit_from_log {
            let mut logger = self.logger.borrow_mut();
            logger.log_input_line(input.as_bytes());
            logger.log_error(handler);
        }
        outcome.into()
    }

    pub fn log_note(&self, note: &[u8]) {
        self.logger.borrow_mut().log_note(note);
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
        self.variables.borrow_mut().save(writer)
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

    pub fn set_plugin_enabled(&mut self, index: PluginIndex, enabled: bool) -> bool {
        let Some(plugin) = self.plugins.get_mut(index) else {
            return false;
        };
        plugin.disabled = !enabled;
        true
    }

    pub fn find_sender<T: SendIterable>(
        &self,
        index: PluginIndex,
        label: &str,
    ) -> Option<Ref<'_, T>> {
        self.senders::<T>(index)
            .find(|sender| sender.as_ref().label == label)
    }

    pub fn find_sender_mut<T: SendIterable>(
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
        self.find_sender_mut::<T>(index, label)?.as_mut().enabled = enabled;
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
        senders.remove(pos);
        Ok(())
    }

    pub fn remove_senders<T: SendIterable>(&self, index: PluginIndex, group: &str) -> usize {
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
    ) -> Result<RefMut<'_, SortOnDrop<T>>, XmlError> {
        let mut senders = T::from_xml_str(xml)?;
        Ok(self.world.import_senders(&mut senders))
    }

    pub fn export_world_senders<T: SendIterable>(&self) -> Result<String, XmlSerError> {
        T::to_xml_string(&*T::from_world(&self.world).borrow())
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
