use std::{mem, slice};

use crate::handler::{Handler, SendHandler};
use crate::plugins::{AliasOutcome, PluginEngine};
use crate::world::World;
use enumeration::EnumSet;
use mud_transformer::{
    EffectFragment, Output, OutputDrain, OutputFragment, Tag, TextFragment, TextStyle,
    TransformerConfig,
};
use smushclient_plugins::{Plugin, Sendable};

#[derive(Clone, Debug, Default, PartialEq)]
pub struct SmushClient {
    output_buf: Vec<Output>,
    line_text: String,
    plugins: PluginEngine,
    supported_tags: EnumSet<Tag>,
    world: World,
}

impl SmushClient {
    pub fn new(world: World, supported_tags: EnumSet<Tag>) -> Self {
        let mut plugins = PluginEngine::new();
        plugins.set_world_plugin(world.world_plugin());
        Self {
            output_buf: Vec::new(),
            line_text: String::new(),
            plugins,
            supported_tags,
            world,
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
    pub fn set_world(&mut self, world: World) -> TransformerConfig {
        self.plugins.set_world_plugin(world.world_plugin());
        self.world = world;
        self.config()
    }

    pub fn receive<H: Handler>(&mut self, mut output: OutputDrain, handler: &mut H) {
        if self.output_buf.is_empty() {
            receive_lines(
                &mut output,
                handler,
                &mut self.plugins,
                &mut self.line_text,
                &self.world,
            );
            self.output_buf.extend(output);
            return;
        }
        self.output_buf.extend(output);
        let Some(last_break) = self.output_buf.iter().rposition(is_break) else {
            return;
        };
        let mut output = self.output_buf.drain(..=last_break);
        receive_lines(
            &mut output,
            handler,
            &mut self.plugins,
            &mut self.line_text,
            &self.world,
        );
    }

    pub fn alias<H: SendHandler>(&mut self, input: &str, handler: &mut H) -> AliasOutcome {
        self.plugins.alias(input, handler).into()
    }

    pub fn load_plugins<I: IntoIterator<Item = Plugin>>(&mut self, iter: I) {
        self.plugins.load_plugins(iter);
    }

    pub fn plugins(&self) -> slice::Iter<Plugin> {
        self.plugins.iter()
    }

    pub fn set_enabled<T: Sendable>(&mut self, label: &str, enabled: bool) -> bool {
        let Some(sender) = self
            .plugins
            .find_by_mut(|item: &T| item.as_ref().label == label)
            .next()
        else {
            return false;
        };
        sender.as_mut().enabled = enabled;
        true
    }

    pub fn set_group_enabled<T: Sendable>(&mut self, group: &str, enabled: bool) -> bool {
        let mut found_group = false;
        for sender in self
            .plugins
            .find_by_mut(|item: &T| item.as_ref().group == group)
        {
            found_group = true;
            sender.as_mut().enabled = enabled;
        }
        found_group
    }
}

fn is_nonvisual_output(fragment: &OutputFragment) -> bool {
    matches!(
        fragment,
        OutputFragment::Effect(EffectFragment::Beep) | OutputFragment::Telnet(_)
    )
}

fn is_break(output: &Output) -> bool {
    matches!(
        output.fragment,
        OutputFragment::Hr | OutputFragment::LineBreak | OutputFragment::PageBreak
    )
}

fn receive_lines<H: Handler>(
    output: &mut OutputDrain,
    handler: &mut H,
    plugins: &mut PluginEngine,
    line_text: &mut String,
    world: &World,
) {
    loop {
        let slice = output.as_slice();
        if slice.is_empty() {
            return;
        }
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
        if until == 0 {
            return;
        }
        let trigger_effects = plugins.trigger(line_text, handler);
        if trigger_effects.omit_from_output {
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
                    alter_text_output(text, world);
                }
                handler.display(output);
            }
        }
    }
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
