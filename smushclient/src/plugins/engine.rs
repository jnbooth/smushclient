use core::str;
use std::cell::RefCell;
use std::collections::HashSet;
use std::fs::File;
use std::io::{self};
use std::ops::{Deref, DerefMut};
use std::path::Path;
use std::{slice, vec};

use arboard::Clipboard;
use mud_transformer::Output;
use rodio::Decoder;
use rodio::mixer::Mixer;
use smushclient_plugins::{Alias, LoadError, Plugin, PluginIndex, Reaction, SendTarget, Trigger};

use super::effects::CommandSource;
use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use super::error::LoadFailure;
use super::iter::ReactionIterable;
use crate::client::PluginVariables;
use crate::handler::{Handler, HandlerExt};
use crate::plugins::SendRequest;
use crate::world::World;

#[derive(Clone, Debug, PartialEq, Eq)]
pub(crate) struct PluginEngine {
    plugins: Vec<Plugin>,
}

impl Default for PluginEngine {
    fn default() -> Self {
        Self::new()
    }
}

impl PluginEngine {
    pub const fn new() -> Self {
        Self {
            plugins: Vec::new(),
        }
    }

    pub fn load_plugins(&mut self, world: &World) -> Result<(), Vec<LoadFailure>> {
        self.plugins.clear();
        self.plugins.push(world.world_plugin());
        let errors: Vec<LoadFailure> = world
            .plugins
            .iter()
            .filter_map(|path| {
                let error = self.load_plugin(path).err()?;
                Some(LoadFailure {
                    error,
                    path: path.clone(),
                })
            })
            .collect();
        self.plugins.sort_unstable();
        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }

    pub fn reinstall_plugin(&mut self, index: PluginIndex) -> Result<usize, LoadError> {
        let plugin = self.plugins.get_mut(index).ok_or(io::ErrorKind::NotFound)?;
        let path = plugin.metadata.path.clone();
        plugin.disabled.set(true);
        *plugin = Plugin::load(&path)?;
        let id = plugin.metadata.id.clone();
        self.plugins.sort_unstable();
        if self.plugins[index].metadata.id == id {
            return Ok(index);
        }
        Ok(self
            .plugins
            .iter()
            .position(|plugin| plugin.metadata.id == id)
            .unwrap())
    }

    pub fn add_plugin<P: AsRef<Path>>(
        &mut self,
        path: P,
    ) -> Result<(PluginIndex, &Plugin), LoadError> {
        let path = path.as_ref();
        if self
            .plugins
            .iter()
            .any(|plugin| plugin.metadata.path == path)
        {
            return Err(io::Error::from(io::ErrorKind::AlreadyExists).into());
        }
        self.load_plugin(path)?;
        self.plugins.sort_unstable();
        Ok(self
            .plugins
            .iter()
            .enumerate()
            .find(|(_, plugin)| plugin.metadata.path == path)
            .unwrap())
    }

    fn load_plugin(&mut self, path: &Path) -> Result<&Plugin, LoadError> {
        let plugin = Plugin::load(path)?;
        self.plugins.push(plugin);
        Ok(self.plugins.last().unwrap())
    }

    pub fn remove_plugin(&mut self, index: PluginIndex) -> Option<Plugin> {
        if index > self.plugins.len() {
            None
        } else {
            Some(self.plugins.remove(index))
        }
    }

    pub fn set_world_plugin(&mut self, plugin: Plugin) {
        if let Some(world_plugin) = self
            .plugins
            .iter_mut()
            .find(|plugin| plugin.metadata.is_world_plugin)
        {
            *world_plugin = plugin;
        } else {
            self.plugins.push(plugin);
        }
        self.plugins.sort_unstable();
    }

    pub fn supported_protocols(&self) -> HashSet<u8> {
        self.plugins
            .iter()
            .flat_map(|plugin| plugin.metadata.protocols.iter())
            .copied()
            .collect()
    }

    pub fn alias<H: Handler>(
        &self,
        line: &str,
        source: CommandSource,
        world: &World,
        variables: &RefCell<PluginVariables>,
        audio: &Mixer,
        handler: &mut H,
    ) -> AliasEffects {
        let mut effects = AliasEffects::new(world, source);
        let echo = match source {
            CommandSource::Hotkey => world.echo_hotkey_in_output_window,
            CommandSource::Link => world.echo_hyperlink_in_output_window,
            CommandSource::User => true,
        };
        self.process_matches::<Alias, _>(
            line,
            &[],
            echo,
            world,
            variables,
            audio,
            handler,
            &mut effects,
        );
        effects
    }

    pub fn trigger<H: Handler>(
        &self,
        line: &str,
        output: &[Output],
        world: &World,
        variables: &RefCell<PluginVariables>,
        audio: &Mixer,
        handler: &mut H,
    ) -> TriggerEffects {
        let mut effects = TriggerEffects::new();
        self.process_matches::<Trigger, _>(
            line,
            output,
            true,
            world,
            variables,
            audio,
            handler,
            &mut effects,
        );
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
        world: &World,
        variables: &RefCell<PluginVariables>,
        audio: &Mixer,
        handler: &mut H,
        effects: &mut T::Effects,
    ) {
        if !T::enabled(world) {
            return;
        }
        let mut text_buf = String::new();
        let mut reaction_buf = Reaction::default();
        let mut style = SpanStyle::null();
        let mut has_style = false;

        for (plugin_index, plugin) in self.plugins.iter().enumerate() {
            if plugin.disabled.get() {
                continue;
            }
            let enable_scripts = !plugin.metadata.is_world_plugin || world.enable_scripts;
            let senders = if plugin.metadata.is_world_plugin {
                T::from_world(world)
            } else {
                T::from_plugin(plugin)
            };
            for sender in senders.scan() {
                let mut matched = false;
                let regex = {
                    let Some(sender) = sender.borrow() else {
                        continue;
                    };
                    let reaction = sender.reaction();
                    if !reaction.enabled {
                        continue;
                    }
                    reaction.regex.clone()
                };
                text_buf.clear();
                for captures in regex.captures_iter(line).filter_map(Result::ok) {
                    if !matched {
                        matched = true;
                        if T::AFFECTS_STYLE {
                            let Some(sender) = sender.borrow() else {
                                break;
                            };
                            style = sender.style();
                            has_style = !style.is_null();
                        }
                    }
                    if has_style && let Some(capture) = captures.get(0) {
                        handler.apply_styles(capture.start()..capture.end(), style);
                    }
                    let send_request = {
                        let Some(sender) = sender.borrow() else {
                            break;
                        };
                        if let Some(clipboard_arg) = sender.clipboard_arg()
                            && let Some(capture) = captures.get(clipboard_arg.get().into())
                            && let Ok(mut clipboard) = Clipboard::new()
                        {
                            clipboard.set_text(capture.as_str()).ok();
                        }
                        let reaction = sender.reaction();
                        if reaction.send_to == SendTarget::Variable {
                            variables.borrow_mut().set_variable(
                                &plugin.metadata.id,
                                reaction.variable.as_bytes().to_vec(),
                                reaction
                                    .expand_text(&mut text_buf, &captures)
                                    .as_bytes()
                                    .to_vec(),
                            );
                            false
                        } else if enable_scripts || !reaction.send_to.is_script() {
                            reaction_buf.clone_from(reaction);
                            true
                        } else {
                            false
                        }
                    };
                    if send_request {
                        handler.send(SendRequest {
                            plugin: plugin_index,
                            send_to: reaction_buf.send_to,
                            echo: permit_echo && reaction_buf.should_echo(),
                            log: !reaction_buf.omit_from_log,
                            text: reaction_buf.expand_text(&mut text_buf, &captures),
                            destination: reaction_buf.destination(),
                        });
                    }
                    // fresh borrow in case the handler changed the reaction's settings
                    if !sender
                        .borrow()
                        .is_some_and(|sender| sender.reaction().repeats)
                    {
                        break;
                    }
                }
                if !matched {
                    continue;
                }
                let send_script = enable_scripts && {
                    let Some(sender) = sender.borrow() else {
                        continue;
                    };
                    let reaction = sender.reaction();
                    if reaction.script.is_empty() {
                        false
                    } else {
                        reaction_buf.clone_from(reaction);
                        true
                    }
                };
                if send_script {
                    handler.send_scripts(plugin_index, &reaction_buf, line, output);
                }
                let (one_shot, keep_evaluating) = {
                    let Some(sender) = sender.borrow() else {
                        continue;
                    };
                    if let Some(sound) = sender.sound()
                        && world.enable_trigger_sounds
                        && let Ok(file) = File::open(sound)
                        && let Ok(decoder) = Decoder::try_from(file)
                    {
                        audio.add(decoder);
                    }
                    sender.add_effects(effects);
                    let reaction = sender.reaction();
                    (reaction.one_shot, reaction.keep_evaluating)
                };
                if one_shot {
                    sender.remove();
                }
                if !keep_evaluating {
                    break;
                }
            }
        }
    }
}

impl Deref for PluginEngine {
    type Target = [Plugin];

    fn deref(&self) -> &Self::Target {
        &self.plugins
    }
}

impl DerefMut for PluginEngine {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.plugins
    }
}

impl IntoIterator for PluginEngine {
    type Item = Plugin;

    type IntoIter = vec::IntoIter<Plugin>;

    fn into_iter(self) -> Self::IntoIter {
        self.plugins.into_iter()
    }
}

impl<'a> IntoIterator for &'a PluginEngine {
    type Item = &'a Plugin;

    type IntoIter = slice::Iter<'a, Plugin>;

    fn into_iter(self) -> Self::IntoIter {
        self.plugins.iter()
    }
}

impl<'a> IntoIterator for &'a mut PluginEngine {
    type Item = &'a mut Plugin;

    type IntoIter = slice::IterMut<'a, Plugin>;

    fn into_iter(self) -> Self::IntoIter {
        self.plugins.iter_mut()
    }
}
