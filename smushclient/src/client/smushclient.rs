use std::ffi::c_char;
use std::io::{Read, Write};
use std::path::Path;
use std::{env, mem, slice};

use super::variables::PluginVariables;
use crate::handler::{Handler, SendHandler};
use crate::plugins::{AliasOutcome, LoadFailure, PluginEngine};
use crate::world::{PersistError, World};
use crate::LoadError;
use enumeration::EnumSet;
use mud_transformer::{
    EffectFragment, OutputDrain, OutputFragment, Tag, TextFragment, TextStyle, TransformerConfig,
};
use smushclient_plugins::{Plugin, PluginIndex, Sendable};

#[derive(Clone, Debug, Default, PartialEq)]
pub struct SmushClient {
    line_text: String,
    pub(crate) plugins: PluginEngine,
    supported_tags: EnumSet<Tag>,
    world: World,
    variables: PluginVariables,
}

impl SmushClient {
    pub fn new(world: World, supported_tags: EnumSet<Tag>) -> Self {
        let mut plugins = PluginEngine::new();
        plugins.set_world_plugin(world.world_plugin());
        Self {
            line_text: String::new(),
            plugins,
            supported_tags,
            world,
            variables: PluginVariables::new(),
        }
    }

    pub fn set_supported_tags(&mut self, supported_tags: EnumSet<Tag>) {
        self.supported_tags = supported_tags;
    }

    pub fn config(&self) -> TransformerConfig {
        TransformerConfig {
            will: self.plugins.supported_protocols(),
            supports: self.supported_tags,
            ..TransformerConfig::from(&self.world)
        }
    }

    pub fn world(&self) -> &World {
        &self.world
    }

    #[must_use = "config changes must be applied to the transformer"]
    pub fn set_world_and_plugins(&mut self, world: World) -> TransformerConfig {
        self.plugins.set_world_plugin(world.world_plugin());
        self.world = world;
        self.config()
    }

    #[must_use = "config changes must be applied to the transformer"]
    pub fn set_world(&mut self, mut world: World) -> TransformerConfig {
        mem::swap(&mut world.plugins, &mut self.world.plugins);
        self.plugins.set_world_plugin(world.world_plugin());
        self.world = world;
        self.config()
    }

    pub fn receive<H: Handler>(&mut self, mut output: OutputDrain, handler: &mut H) {
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
            let trigger_effects = self.plugins.trigger(&self.line_text, handler);
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
        }
    }

    pub fn alias<H: SendHandler>(&mut self, input: &str, handler: &mut H) -> AliasOutcome {
        self.plugins.alias(input, handler).into()
    }

    #[must_use = "config changes must be applied to the transformer"]
    pub fn load_plugins(&mut self) -> Result<TransformerConfig, Vec<LoadFailure>> {
        self.plugins.load_plugins(&self.world)?;
        Ok(self.config())
    }

    pub fn add_plugin<P: AsRef<Path>>(&mut self, path: P) -> Result<&Plugin, LoadError> {
        let path = path.as_ref();
        let path = env::current_dir()
            .ok()
            .and_then(|cwd| path.strip_prefix(cwd).ok())
            .unwrap_or(path);
        let index = self.plugins.add_plugin(path)?.0;
        self.update_world_plugins();
        Ok(self.plugins.plugin(index).unwrap())
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
        let plugin_id = &self.plugins.plugin(index)?.metadata.id;
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
        let plugin_id = &self.plugins.plugin(index)?.metadata.id;
        self.variables.get_variable(plugin_id, key)
    }

    pub fn set_variable(
        &mut self,
        index: PluginIndex,
        key: Vec<c_char>,
        value: Vec<c_char>,
    ) -> bool {
        let Some(plugin) = self.plugins.plugin(index) else {
            return false;
        };
        let plugin_id = &plugin.metadata.id;
        self.variables.set_variable(plugin_id, key, value);
        true
    }

    pub fn set_group_enabled<T: Sendable>(&mut self, group: &str, enabled: bool) -> bool {
        let mut found_group = false;
        for sender in self
            .plugins
            .indexer_mut::<T>()
            .find_by_mut(|item| item.as_ref().group == group)
        {
            found_group = true;
            sender.as_mut().enabled = enabled;
        }
        found_group
    }

    pub fn set_plugin_enabled(&mut self, index: PluginIndex, enabled: bool) -> bool {
        if enabled {
            self.plugins.enable_plugin(index)
        } else {
            self.plugins.disable_plugin(index)
        }
    }

    pub fn set_sender_enabled<T: Sendable>(&mut self, label: &str, enabled: bool) -> bool {
        let Some(sender) = self
            .plugins
            .indexer_mut::<T>()
            .find_by_mut(|item| item.as_ref().label == label)
            .next()
        else {
            return false;
        };
        sender.as_mut().enabled = enabled;
        true
    }

    pub fn sender_exists<T: Sendable>(&self, label: &str) -> bool {
        self.plugins
            .indexer::<T>()
            .find_by(|item| item.as_ref().label == label)
            .next()
            .is_some()
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
