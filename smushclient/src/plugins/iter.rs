use smushclient_plugins::{
    Alias, CursorVec, Plugin, Reaction, Sender, Timer, Trigger, XmlError, XmlSerError,
};

use super::effects::{AliasEffects, SpanStyle, TriggerEffects};
use crate::world::World;

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Eq + Ord + Sized {
    fn from_xml_str(s: &str) -> Result<Vec<Self>, XmlError>;
    fn to_xml_string<'a, I: IntoIterator<Item = &'a Self>>(iter: I) -> Result<String, XmlSerError>
    where
        Self: 'a;
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

macro_rules! impl_send_iterable {
    ($t:ty, $i:ident, $e:ident) => {
        impl SendIterable for $t {
            fn from_xml_str(s: &str) -> Result<Vec<Self>, XmlError> {
                Self::from_xml_str(s)
            }
            fn to_xml_string<'a, I: IntoIterator<Item = &'a Self>>(
                iter: I,
            ) -> Result<String, XmlSerError> {
                Self::to_xml_string(iter)
            }
            fn enabled(world: &World) -> bool {
                world.$e
            }
            fn from_plugin(plugin: &Plugin) -> &CursorVec<Self> {
                &plugin.$i
            }
            fn from_world(plugin: &World) -> &CursorVec<Self> {
                &plugin.$i
            }
        }
    };
}

impl_send_iterable!(Alias, aliases, enable_aliases);
impl_send_iterable!(Timer, timers, enable_timers);
impl_send_iterable!(Trigger, triggers, enable_triggers);

pub trait ReactionIterable: SendIterable {
    const AFFECTS_STYLE: bool;
    type Effects;

    fn reaction(&self) -> &Reaction;
    fn sound(&self) -> Option<&str>;
    fn style(&self) -> SpanStyle;
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

    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}
