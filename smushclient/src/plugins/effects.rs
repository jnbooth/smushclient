use flagset::FlagSet;
use mud_transformer::TextStyle;
use mud_transformer::mxp::RgbColor;
use smushclient_plugins::{Alias, Trigger};

use crate::world::World;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum CommandSource {
    #[default]
    User,
    Hotkey,
    Link,
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub(crate) struct AliasEffects {
    pub echo: bool,
    pub omit_from_command_history: bool,
    pub matched: bool,
}

impl AliasEffects {
    pub const fn new(world: &World, source: CommandSource) -> Self {
        match source {
            CommandSource::User => Self {
                echo: true,
                omit_from_command_history: false,
                matched: false,
            },
            CommandSource::Hotkey => Self {
                echo: world.echo_hotkey_in_output_window,
                omit_from_command_history: !world.hotkey_adds_to_command_history,
                matched: false,
            },
            CommandSource::Link => Self {
                echo: world.echo_hyperlink_in_output_window,
                omit_from_command_history: !world.hyperlink_adds_to_command_history,
                matched: false,
            },
        }
    }

    pub fn add_effects(&mut self, alias: &Alias) {
        if !self.matched {
            self.echo = false;
            self.matched = true;
        }
        self.omit_from_command_history |= alias.omit_from_command_history;
        self.echo |= alias.echo_alias;
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AliasOutcome {
    pub echo: bool,
    pub remember: bool,
    pub send: bool,
}

impl Default for AliasOutcome {
    fn default() -> Self {
        Self {
            echo: true,
            remember: true,
            send: true,
        }
    }
}

impl From<AliasEffects> for AliasOutcome {
    fn from(value: AliasEffects) -> Self {
        Self {
            echo: value.echo,
            remember: !value.omit_from_command_history,
            send: !value.matched,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub(crate) struct TriggerEffects {
    pub omit_from_output: bool,
    pub omit_from_log: bool,
}

impl Default for TriggerEffects {
    fn default() -> Self {
        Self::new()
    }
}

impl TriggerEffects {
    pub const fn new() -> Self {
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

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq)]
pub struct SpanStyle {
    pub flags: FlagSet<TextStyle>,
    pub foreground: Option<RgbColor>,
    pub background: Option<RgbColor>,
}

impl SpanStyle {
    pub const fn null() -> Self {
        Self {
            flags: FlagSet::empty(),
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
        let mut flags = FlagSet::empty();
        if trigger.make_bold {
            flags |= TextStyle::Bold;
        }
        if trigger.make_italic {
            flags |= TextStyle::Italic;
        }
        if trigger.make_underline {
            flags |= TextStyle::Underline;
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
