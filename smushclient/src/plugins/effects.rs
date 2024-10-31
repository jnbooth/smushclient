use enumeration::{Enum, EnumSet};
use mud_transformer::mxp::RgbColor;
use mud_transformer::TextStyle;
use smushclient_plugins::{Alias, Trigger};

use crate::world::World;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Enum)]
pub enum CommandSource {
    Hotkey,
    Link,
    User,
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AliasEffects {
    pub keep_evaluating: bool,
    pub omit_from_command_history: bool,
    pub omit_from_log: bool,
    pub omit_from_output: bool,
    pub suppress: bool,
}

impl AliasEffects {
    pub const fn new(world: &World, source: CommandSource) -> Self {
        match source {
            CommandSource::Hotkey => Self {
                keep_evaluating: true,
                omit_from_command_history: !world.hotkey_adds_to_command_history,
                omit_from_log: false,
                omit_from_output: !world.echo_hotkey_in_output_window,
                suppress: false,
            },
            CommandSource::Link => Self {
                keep_evaluating: true,
                omit_from_command_history: !world.hyperlink_adds_to_command_history,
                omit_from_log: false,
                omit_from_output: !world.echo_hyperlink_in_output_window,
                suppress: false,
            },
            CommandSource::User => Self {
                keep_evaluating: true,
                omit_from_command_history: false,
                omit_from_log: false,
                omit_from_output: false,
                suppress: false,
            },
        }
    }

    pub fn add_effects(&mut self, alias: &Alias) {
        self.keep_evaluating &= alias.keep_evaluating;
        self.omit_from_command_history |= alias.omit_from_command_history;
        self.omit_from_log |= alias.omit_from_log;
        self.omit_from_output |= alias.omit_from_output;
        self.suppress |= !alias.keep_evaluating;
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AliasOutcome {
    pub remember: bool,
    pub send: bool,
}

impl From<AliasEffects> for AliasOutcome {
    fn from(value: AliasEffects) -> Self {
        Self {
            remember: !value.omit_from_command_history,
            send: !value.suppress,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct TriggerEffects {
    pub omit_from_output: bool,
    pub omit_from_log: bool,
}

impl Default for TriggerEffects {
    fn default() -> Self {
        Self::new()
    }
}

impl TriggerEffects {
    pub fn new() -> Self {
        Self {
            omit_from_output: false,
            omit_from_log: false,
        }
    }

    pub fn add_effects(&mut self, trigger: &Trigger) {
        self.omit_from_log |= trigger.omit_from_log;
        self.omit_from_output |= trigger.omit_from_output;
    }

    /*
    pub fn apply(&self, fragment: &mut TextFragment, world: &World) {
        if fragment.flags.contains(TextStyle::Inverse) {
            mem::swap(&mut fragment.foreground, &mut fragment.background);
        }
        if let Some(foreground) = self.foreground {
            fragment.foreground = foreground;
        }
        if let Some(background) = self.background {
            fragment.background = background;
        }
        if self.make_bold {
            fragment.flags.insert(TextStyle::Bold);
        } else if !world.show_bold {
            fragment.flags.remove(TextStyle::Bold);
        }
        if self.make_italic {
            fragment.flags.insert(TextStyle::Italic);
        } else if !world.show_italic {
            fragment.flags.remove(TextStyle::Italic);
        }
        if self.make_underline {
            fragment.flags.insert(TextStyle::Underline);
        } else if !world.show_underline {
            fragment.flags.remove(TextStyle::Underline);
        }
        if fragment.action.is_none() {
            return;
        }
        if world.underline_hyperlinks {
            fragment.flags.insert(TextStyle::Underline);
        }
        if world.mud_can_change_link_colour {
            return;
        }
        fragment.foreground = if world.use_custom_link_colour {
            world.hyperlink_colour
        } else {
            RgbColor::rgb(43, 121, 162)
        }
    }
    */
}

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct SpanStyle {
    pub flags: EnumSet<TextStyle>,
    pub foreground: Option<RgbColor>,
    pub background: Option<RgbColor>,
}

impl Default for SpanStyle {
    fn default() -> Self {
        Self::null()
    }
}

impl SpanStyle {
    pub const fn null() -> Self {
        Self {
            flags: EnumSet::new(),
            foreground: None,
            background: None,
        }
    }

    pub fn is_null(&self) -> bool {
        self.flags.is_empty() && self.foreground.is_none() && self.background.is_none()
    }
}

impl From<&Trigger> for SpanStyle {
    fn from(trigger: &Trigger) -> Self {
        let mut flags = EnumSet::new();
        if trigger.make_bold {
            flags.insert(TextStyle::Bold);
        }
        if trigger.make_italic {
            flags.insert(TextStyle::Italic);
        }
        if trigger.make_underline {
            flags.insert(TextStyle::Underline);
        }
        Self {
            flags,
            foreground: if trigger.change_foreground {
                Some(trigger.foreground_color)
            } else {
                None
            },
            background: if trigger.change_background {
                Some(trigger.background_color)
            } else {
                None
            },
        }
    }
}
