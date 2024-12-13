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
    pub omit_from_command_history: bool,
    pub omit_from_log: bool,
    pub omit_from_output: bool,
    pub suppress: bool,
}

impl AliasEffects {
    pub const fn new(world: &World, source: CommandSource) -> Self {
        match source {
            CommandSource::Hotkey => Self {
                omit_from_command_history: !world.hotkey_adds_to_command_history,
                omit_from_log: false,
                omit_from_output: !world.echo_hotkey_in_output_window,
                suppress: false,
            },
            CommandSource::Link => Self {
                omit_from_command_history: !world.hyperlink_adds_to_command_history,
                omit_from_log: false,
                omit_from_output: !world.echo_hyperlink_in_output_window,
                suppress: false,
            },
            CommandSource::User => Self {
                omit_from_command_history: false,
                omit_from_log: false,
                omit_from_output: false,
                suppress: false,
            },
        }
    }

    pub fn add_effects(&mut self, alias: &Alias) {
        self.omit_from_command_history |= alias.omit_from_command_history;
        self.omit_from_log |= alias.omit_from_log;
        self.omit_from_output |= alias.omit_from_output;
        self.suppress |= !alias.keep_evaluating;
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AliasOutcome {
    pub display: bool,
    pub remember: bool,
    pub send: bool,
}

impl From<AliasEffects> for AliasOutcome {
    fn from(value: AliasEffects) -> Self {
        Self {
            display: !value.omit_from_output,
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
