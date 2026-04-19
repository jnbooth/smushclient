use smushclient_plugins::{Alias, Captures, PluginSender, Reaction, Trigger};

use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use crate::world::WorldConfig;

pub(crate) trait PluginReaction: PluginSender {
    const AFFECTS_STYLE: bool;
    type Effects;

    fn enabled(world: &WorldConfig) -> bool;
    fn echo_input(&self) -> bool;
    fn reaction(&self) -> &Reaction;
    fn sound(&self) -> Option<&str>;
    fn style(&self) -> SpanStyle;
    fn clipboard_text<'a>(&self, captures: &'a Captures) -> Option<&'a str>;
    fn add_effects(&self, effects: &mut Self::Effects);
}

impl PluginReaction for Alias {
    const AFFECTS_STYLE: bool = false;
    type Effects = AliasEffects;

    fn enabled(world: &WorldConfig) -> bool {
        world.enable_aliases
    }

    fn echo_input(&self) -> bool {
        self.echo_alias
    }
    fn reaction(&self) -> &Reaction {
        &self.reaction
    }
    fn sound(&self) -> Option<&str> {
        None
    }
    fn style(&self) -> SpanStyle {
        SpanStyle::null()
    }
    fn clipboard_text<'a>(&self, _: &'a Captures) -> Option<&'a str> {
        None
    }
    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}

impl PluginReaction for Trigger {
    const AFFECTS_STYLE: bool = true;
    type Effects = TriggerEffects;

    fn enabled(world: &WorldConfig) -> bool {
        world.enable_triggers
    }

    fn echo_input(&self) -> bool {
        false
    }
    fn reaction(&self) -> &Reaction {
        &self.reaction
    }
    fn sound(&self) -> Option<&str> {
        if self.sound.is_empty() {
            None
        } else {
            Some(&self.sound)
        }
    }
    fn style(&self) -> SpanStyle {
        SpanStyle::from(self)
    }
    fn clipboard_text<'a>(&self, captures: &'a Captures) -> Option<&'a str> {
        if self.clipboard_arg == 0 {
            return None;
        }
        Some(captures.get(self.clipboard_arg.into())?.as_str())
    }
    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}
