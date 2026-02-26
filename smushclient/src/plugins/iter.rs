use std::num::NonZero;

use smushclient_plugins::{Alias, PluginItem, Reaction, Timer, Trigger, XmlIterable};

use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use crate::world::WorldConfig;

pub trait SendIterable: XmlIterable + PluginItem {
    fn enabled(world: &WorldConfig) -> bool;
}

impl SendIterable for Alias {
    fn enabled(world: &WorldConfig) -> bool {
        world.enable_aliases
    }
}

impl SendIterable for Timer {
    fn enabled(world: &WorldConfig) -> bool {
        world.enable_timers
    }
}

impl SendIterable for Trigger {
    fn enabled(world: &WorldConfig) -> bool {
        world.enable_triggers
    }
}

pub(crate) trait ReactionIterable: SendIterable {
    const AFFECTS_STYLE: bool;
    type Effects;

    fn echo_input(&self) -> bool;
    fn reaction(&self) -> &Reaction;
    fn sound(&self) -> Option<&str>;
    fn style(&self) -> SpanStyle;
    fn clipboard_arg(&self) -> Option<NonZero<u8>>;
    fn add_effects(&self, effects: &mut Self::Effects);
}

impl ReactionIterable for Alias {
    const AFFECTS_STYLE: bool = false;
    type Effects = AliasEffects;

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
    fn clipboard_arg(&self) -> Option<NonZero<u8>> {
        None
    }
    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}

impl ReactionIterable for Trigger {
    const AFFECTS_STYLE: bool = true;
    type Effects = TriggerEffects;

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
    fn clipboard_arg(&self) -> Option<NonZero<u8>> {
        NonZero::new(self.clipboard_arg)
    }
    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}
