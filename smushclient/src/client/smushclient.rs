use std::cmp::Ordering;
use std::ffi::c_char;
use std::io::{self, Read, Write};
use std::path::Path;
use std::{env, mem, slice};

use super::logger::Logger;
use super::variables::PluginVariables;
use crate::handler::{Handler, SendHandler};
use crate::plugins::{AliasOutcome, LoadFailure, PluginEngine, SendIterable};
use crate::world::{PersistError, World};
use crate::LoadError;
use enumeration::EnumSet;
use mud_transformer::{
    EffectFragment, OutputFragment, Tag, TextFragment, TextStyle, Transformer, TransformerConfig,
};
use smushclient_plugins::{Plugin, PluginIndex};
#[cfg(feature = "async")]
use tokio::io::{AsyncRead, AsyncReadExt, AsyncWrite, AsyncWriteExt};

const BUF_LEN: usize = 1024 * 20;
const BUF_MIDPOINT: usize = BUF_LEN / 2;

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
            logger: Logger::Closed,
            plugins,
            read_buf: vec![0; BUF_LEN],
            supported_tags,
            transformer,
            variables: PluginVariables::new(),
            world,
        }
    }

    pub fn set_supported_tags(&mut self, supported_tags: EnumSet<Tag>) {
        self.supported_tags = supported_tags;
    }

    fn update_config(&mut self) {
        self.transformer.set_config(TransformerConfig {
            will: self.plugins.supported_protocols(),
            supports: self.supported_tags,
            ..TransformerConfig::from(&self.world)
        });
    }

    pub fn world(&self) -> &World {
        &self.world
    }

    pub fn set_world_and_plugins(&mut self, world: World) {
        self.plugins.set_world_plugin(world.world_plugin());
        self.world = world;
        self.update_config();
    }

    pub fn set_world(&mut self, mut world: World) {
        mem::swap(&mut world.plugins, &mut self.world.plugins);
        self.plugins.set_world_plugin(world.world_plugin());
        self.world = world;
        self.update_config();
    }

    pub fn open_log(&mut self) -> io::Result<()> {
        if self.world.auto_log_file_name != self.last_log_file_name {
            self.last_log_file_name = self.world.auto_log_file_name.clone();
        } else if matches!(self.logger, Logger::Open(..)) {
            return Ok(());
        }
        self.last_log_file_name = self.world.auto_log_file_name.clone();
        self.logger = Logger::open(&self.world)?;
        Ok(())
    }

    pub fn read<R: Read>(&mut self, mut reader: R) -> io::Result<usize> {
        let mut total_read = 0;
        loop {
            let n = reader.read(&mut self.read_buf[..BUF_MIDPOINT])?;
            if n == 0 {
                return Ok(total_read);
            }
            let (received, buf) = self.read_buf.split_at_mut(n);
            self.logger.log_raw(received, &self.world);
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
            self.logger.log_raw(buf, &self.world);
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

    pub fn flush_output<H: Handler>(&mut self, handler: &mut H) {
        self.logger.log_error(handler);
        let mut output = self.transformer.flush_output();
        loop {
            let slice = output.as_slice();
            if slice.is_empty() {
                return;
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
            let trigger_effects =
                self.plugins
                    .trigger(&self.line_text, &mut self.world, &slice[..until], handler);
            if !handler.permit_line(&self.line_text) || trigger_effects.omit_from_output {
                for _ in 0..until {
                    let output = output.next().unwrap();
                    if is_nonvisual_output(&output.fragment) {
                        handler.display(output);
                    }
                }
            } else {
                for _ in 0..until {
                    let mut output = output.next().unwrap();
                    if let OutputFragment::Text(text) = &mut output.fragment {
                        alter_text_output(text, &self.world);
                    }
                    handler.display(output);
                }
            }
            if !trigger_effects.omit_from_log {
                self.logger
                    .log_output_line(self.line_text.as_bytes(), &self.world);
            }
        }
    }

    pub fn alias<H: SendHandler>(&mut self, input: &str, handler: &mut H) -> AliasOutcome {
        let outcome = self.plugins.alias(input, &mut self.world, handler);
        if !outcome.omit_from_log {
            self.logger.log_input_line(input.as_bytes(), &self.world);
            self.logger.log_error(handler);
        }
        outcome.into()
    }

    pub fn log_note(&mut self, note: &[u8]) {
        self.logger.log_note(note, &self.world);
    }

    pub fn load_plugins(&mut self) -> Result<(), Vec<LoadFailure>> {
        self.plugins.load_plugins(&self.world)?;
        self.update_config();
        Ok(())
    }

    pub fn add_plugin<P: AsRef<Path>>(&mut self, path: P) -> Result<&Plugin, LoadError> {
        let path = path.as_ref();
        let path = env::current_dir()
            .ok()
            .and_then(|cwd| path.strip_prefix(cwd).ok())
            .unwrap_or(path);
        let index = self.plugins.add_plugin(path)?.0;
        self.update_world_plugins();
        Ok(&self.plugins[index])
    }

    pub fn remove_plugin(&mut self, id: &str) -> Option<Plugin> {
        let plugin = self.plugins.remove_plugin(id)?;
        let plugin_path = plugin.metadata.path.as_path();
        self.world.plugins.retain(|path| path != plugin_path);
        Some(plugin)
    }

    pub fn plugins(&self) -> slice::Iter<Plugin> {
        self.plugins.iter()
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

    pub fn get_variable(&self, index: PluginIndex, key: &[c_char]) -> Option<&Vec<c_char>> {
        let plugin_id = &self.plugins.get(index)?.metadata.id;
        self.variables.get_variable(plugin_id, key)
    }

    pub fn set_variable(
        &mut self,
        index: PluginIndex,
        key: Vec<c_char>,
        value: Vec<c_char>,
    ) -> bool {
        let Some(plugin) = self.plugins.get(index) else {
            return false;
        };
        let plugin_id = &plugin.metadata.id;
        self.variables.set_variable(plugin_id, key, value);
        true
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
            .iter()
            .find(|sender| sender.as_ref().label == label)
    }

    pub fn find_sender_mut<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &str,
    ) -> Option<&mut T> {
        self.senders_mut::<T>(index)
            .iter_mut()
            .find(|sender| sender.as_ref().label == label)
    }

    pub fn set_sender_enabled<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        label: &str,
        enabled: bool,
    ) -> bool {
        let Some(sender) = self.find_sender_mut::<T>(index, label) else {
            return false;
        };
        sender.as_mut().enabled = enabled;
        true
    }

    pub fn senders<T: SendIterable>(&self, index: PluginIndex) -> &[T] {
        let plugin = &self.plugins[index];
        if plugin.metadata.is_world_plugin {
            T::from_world(&self.world)
        } else {
            T::from_plugin(plugin)
        }
    }

    pub fn senders_mut<T: SendIterable>(&mut self, index: PluginIndex) -> &mut Vec<T> {
        let plugin = &mut self.plugins[index];
        if plugin.metadata.is_world_plugin {
            T::from_world_mut(&mut self.world)
        } else {
            T::from_plugin_mut(plugin)
        }
    }

    pub fn add_sender<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        sender: T,
    ) -> Result<(usize, &T), usize> {
        let senders = self.senders_mut::<T>(index);
        let label = sender.as_ref().label.as_str();
        if !label.is_empty() {
            if let Some(pos) = senders
                .iter()
                .position(|sender| sender.as_ref().label == label)
            {
                return Err(pos);
            }
        }
        let pos = match senders.binary_search(&sender) {
            Ok(pos) | Err(pos) => pos,
        };
        senders.insert(pos, sender);
        Ok((pos, &senders[pos]))
    }

    pub fn remove_sender<T: SendIterable>(&mut self, index: PluginIndex, label: &str) -> Option<T> {
        let senders = self.senders_mut::<T>(index);
        let pos = senders
            .iter()
            .position(|sender| sender.as_ref().label == label)?;
        Some(senders.remove(pos))
    }

    pub fn remove_senders<T: SendIterable>(&mut self, index: PluginIndex, group: &str) -> usize {
        let senders = self.senders_mut::<T>(index);
        let len = senders.len();
        senders.retain(|sender| sender.as_ref().group != group);
        len - senders.len()
    }

    pub fn replace_sender<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        sender: T,
        replace_at: usize,
    ) -> Result<(usize, &T), usize> {
        let senders = self.senders_mut::<T>(index);
        let label = sender.as_ref().label.as_str();
        if !label.is_empty() {
            if let Some((pos, _)) = senders
                .iter()
                .enumerate()
                .find(|(pos, sender)| sender.as_ref().label == label && *pos != replace_at)
            {
                return Err(pos);
            }
        }
        let pos = match senders.binary_search(&sender) {
            Ok(pos) | Err(pos) => pos,
        };
        if replace_at >= senders.len() {
            senders.insert(pos, sender);
            return Ok((pos, &senders[pos]));
        }
        senders[replace_at] = sender;
        match replace_at.cmp(&pos) {
            Ordering::Less => senders[replace_at..=pos].rotate_left(1),
            Ordering::Equal => (),
            Ordering::Greater => senders[pos..=replace_at].rotate_right(1),
        }
        Ok((pos, &senders[pos]))
    }

    pub fn add_or_replace_sender<T: SendIterable>(
        &mut self,
        index: PluginIndex,
        sender: T,
    ) -> (usize, &T) {
        let senders = self.senders_mut::<T>(index);
        let pos = match senders.binary_search(&sender) {
            Ok(pos) | Err(pos) => pos,
        };
        let label = sender.as_ref().label.as_str();
        if !label.is_empty() {
            if let Some(replace_at) = senders
                .iter()
                .position(|sender| sender.as_ref().label == label)
            {
                senders[replace_at] = sender;
                match replace_at.cmp(&pos) {
                    Ordering::Less => senders[replace_at..=pos].rotate_left(1),
                    Ordering::Equal => (),
                    Ordering::Greater => senders[pos..=replace_at].rotate_right(1),
                }
                return (pos, &senders[pos]);
            }
        }
        senders.insert(pos, sender);
        (pos, &senders[pos])
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

fn is_nonvisual_output(fragment: &OutputFragment) -> bool {
    matches!(
        fragment,
        OutputFragment::Effect(EffectFragment::Beep) | OutputFragment::Telnet(_)
    )
}

fn alter_text_output(fragment: &mut TextFragment, world: &World) {
    if fragment.flags.contains(TextStyle::Inverse) {
        mem::swap(&mut fragment.foreground, &mut fragment.background);
    }
    if !world.show_bold {
        fragment.flags.remove(TextStyle::Bold);
    }
    if !world.show_italic {
        fragment.flags.remove(TextStyle::Italic);
    }
    if !world.show_underline {
        fragment.flags.remove(TextStyle::Underline);
    }
    if fragment.action.is_none() {
        return;
    }
    if world.underline_hyperlinks {
        fragment.flags.insert(TextStyle::Underline);
    }
    if world.use_custom_link_colour {
        fragment.foreground = world.hyperlink_colour;
    }
}
