use std::num::NonZero;

use smushclient_plugins::{
    Alias, CursorVec, Plugin, Reaction, Sender, Timer, Trigger, XmlIterable,
};

use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use crate::world::World;

pub trait SendIterable: XmlIterable + AsRef<Sender> + AsMut<Sender> + Eq + Ord + Sized {
    fn enabled(world: &World) -> bool;
    fn from_plugin(plugin: &Plugin) -> &CursorVec<Self>;
    fn from_world(world: &World) -> &CursorVec<Self>;

    fn from_either<'a>(plugin: &'a Plugin, world: &'a World) -> &'a CursorVec<Self> {
        if plugin.metadata.is_world_plugin {
            Self::from_world(world)
        } else {
            Self::from_plugin(plugin)
        }
    }

    fn assert_unique_label(&self, senders: &CursorVec<Self>) -> Result<(), usize> {
        let label = self.as_ref().label.as_str();
        if label.is_empty() {
            return Ok(());
        }
        match senders.position(|sender| sender.as_ref().label == label) {
            None => Ok(()),
            Some(pos) => Err(pos),
        }
    }
}

impl SendIterable for Alias {
    fn enabled(world: &World) -> bool {
        world.enable_aliases
    }
    fn from_plugin(plugin: &Plugin) -> &CursorVec<Self> {
        &plugin.aliases
    }
    fn from_world(world: &World) -> &CursorVec<Self> {
        &world.aliases
    }
}

impl SendIterable for Timer {
    fn enabled(world: &World) -> bool {
        world.enable_timers
    }
    fn from_plugin(plugin: &Plugin) -> &CursorVec<Self> {
        &plugin.timers
    }
    fn from_world(world: &World) -> &CursorVec<Self> {
        &world.timers
    }
}

impl SendIterable for Trigger {
    fn enabled(world: &World) -> bool {
        world.enable_triggers
    }
    fn from_plugin(plugin: &Plugin) -> &CursorVec<Self> {
        &plugin.triggers
    }
    fn from_world(world: &World) -> &CursorVec<Self> {
        &world.triggers
    }
}

pub trait ReactionIterable: SendIterable {
    const AFFECTS_STYLE: bool;
    type Effects;

    fn reaction(&self) -> &Reaction;
    fn sound(&self) -> Option<&str>;
    fn style(&self) -> SpanStyle;
    fn clipboard_arg(&self) -> Option<NonZero<u8>>;
    fn add_effects(&self, effects: &mut Self::Effects);
}

impl ReactionIterable for Alias {
    const AFFECTS_STYLE: bool = false;
    type Effects = AliasEffects;

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
        self.clipboard_arg
    }
    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}
