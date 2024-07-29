use mud_transformer::mxp::HexColor;
use smushclient_plugins::{Alias, Trigger};

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AliasEffects {
    pub suppress: bool,
}

impl AliasEffects {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn add_effects(&mut self, alias: &Alias) {
        self.suppress |= !alias.keep_evaluating;
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct TriggerEffects {
    pub suppress: bool,
    pub foreground: Option<HexColor>,
    pub background: Option<HexColor>,
    pub make_bold: bool,
    pub make_italic: bool,
    pub make_underline: bool,
}

impl TriggerEffects {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn add_effects(&mut self, trigger: &Trigger) {
        self.suppress |= trigger.omit_from_output;
        if self.suppress {
            return;
        }
        self.make_bold |= trigger.make_bold;
        self.make_italic |= trigger.make_italic;
        self.make_underline |= trigger.make_underline;
        if trigger.change_foreground {
            self.foreground = trigger.foreground_color;
        }
        if trigger.change_background {
            self.background = trigger.background_color;
        }
    }
}
