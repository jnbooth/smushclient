use std::mem;

use mud_transformer::mxp::RgbColor;
use mud_transformer::{TextFragment, TextStyle};
use smushclient_plugins::{Alias, Trigger};

use crate::world::World;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AliasEffects {
    pub keep_evaluating: bool,
    pub omit_from_command_history: bool,
    pub omit_from_log: bool,
    pub omit_from_output: bool,
    pub suppress: bool,
}

impl Default for AliasEffects {
    fn default() -> Self {
        Self::new()
    }
}

impl AliasEffects {
    pub const fn new() -> Self {
        Self {
            keep_evaluating: true,
            omit_from_command_history: false,
            omit_from_log: false,
            omit_from_output: false,
            suppress: false,
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
    pub keep_evaluating: bool,
    pub omit_from_output: bool,
    pub omit_from_log: bool,
    pub foreground: Option<RgbColor>,
    pub background: Option<RgbColor>,
    pub make_bold: bool,
    pub make_italic: bool,
    pub make_underline: bool,
}

impl Default for TriggerEffects {
    fn default() -> Self {
        Self::new()
    }
}

impl TriggerEffects {
    pub fn new() -> Self {
        Self {
            keep_evaluating: true,
            omit_from_output: false,
            omit_from_log: false,
            foreground: None,
            background: None,
            make_bold: false,
            make_italic: false,
            make_underline: false,
        }
    }

    pub fn add_effects(&mut self, trigger: &Trigger) {
        self.keep_evaluating &= trigger.keep_evaluating;
        self.omit_from_log |= trigger.omit_from_log;
        self.omit_from_output |= trigger.omit_from_output;
        if self.omit_from_output {
            return;
        }
        self.make_bold |= trigger.make_bold;
        self.make_italic |= trigger.make_italic;
        self.make_underline |= trigger.make_underline;
        if trigger.change_foreground {
            self.foreground = Some(trigger.foreground_color);
        }
        if trigger.change_background {
            self.background = Some(trigger.background_color);
        }
    }

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
        if world.mud_can_change_link_colour && fragment.foreground != world.ansi_colors[7] {
            return;
        }
        fragment.foreground = if world.use_custom_link_colour {
            world.hyperlink_colour
        } else {
            RgbColor::rgb(43, 121, 162)
        }
    }
}
