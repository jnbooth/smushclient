use mud_transformer::mxp::RgbColor;
use smushclient_plugins::{Alias, Trigger};

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
}
