use std::num::NonZero;

use smushclient_plugins::{Alias, Reaction, SendIterable, Trigger};

use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use crate::world::WorldConfig;

pub(crate) trait ReactionIterable: SendIterable {
    const AFFECTS_STYLE: bool;
    type Effects;

    fn enabled(world: &WorldConfig) -> bool;
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
    fn clipboard_arg(&self) -> Option<NonZero<u8>> {
        NonZero::new(self.clipboard_arg)
    }
    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}
