use smushclient_plugins::{
    Alias, Plugin, PluginIndex, Reaction, Sender, SenderLockError, Timer, Trigger,
};

use super::effects::{AliasEffects, TriggerEffects};
use super::send::SenderAccessError;
use crate::world::World;

pub fn assert_unique_label<T: AsRef<Sender>>(
    sender: &T,
    senders: &[T],
    i: Option<usize>,
) -> Result<(), SenderAccessError> {
    let label = sender.as_ref().label.as_str();
    if label.is_empty() {
        return Ok(());
    }
    if let Some(i) = i {
        match senders
            .iter()
            .enumerate()
            .find(|(pos, sender)| sender.as_ref().label == label && *pos != i)
        {
            Some((pos, _)) => Err(SenderAccessError::LabelConflict(pos)),
            None => Ok(()),
        }
    } else {
        match senders
            .iter()
            .position(|sender| sender.as_ref().label == label)
        {
            Some(pos) => Err(SenderAccessError::LabelConflict(pos)),
            None => Ok(()),
        }
    }
}

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Ord + Sized {
    fn from_plugin(plugin: &Plugin) -> &[Self];
    fn from_plugin_mut(plugin: &mut Plugin) -> &mut Vec<Self>;
    fn from_world(world: &World) -> &[Self];
    fn from_world_mut(world: &mut World) -> &mut Vec<Self>;
    fn is_locked(&self) -> bool {
        self.as_ref().is_locked()
    }
    fn try_unlock(&self) -> Result<(), SenderLockError> {
        self.as_ref().try_unlock()
    }
}

macro_rules! impl_send_iterable {
    ($t:ty, $i:ident) => {
        impl SendIterable for $t {
            fn from_plugin(plugin: &Plugin) -> &[Self] {
                &plugin.$i
            }
            fn from_plugin_mut(plugin: &mut Plugin) -> &mut Vec<Self> {
                &mut plugin.$i
            }
            fn from_world(plugin: &World) -> &[Self] {
                &plugin.$i
            }
            fn from_world_mut(world: &mut World) -> &mut Vec<Self> {
                &mut world.$i
            }
            fn is_locked(&self) -> bool {
                self.send.is_locked()
            }
            fn try_unlock(&self) -> Result<(), SenderLockError> {
                self.send.try_unlock()
            }
        }
    };
}

impl_send_iterable!(Alias, aliases);
impl_send_iterable!(Timer, timers);
impl_send_iterable!(Trigger, triggers);

pub trait ReactionIterable: SendIterable + AsRef<Reaction> + AsMut<Reaction> {
    type Effects: Default + std::fmt::Debug;

    fn add_effects(&self, effects: &mut Self::Effects);

    fn find_matches<'a>(
        plugins: &'a [Plugin],
        world: &'a World,
        line: &str,
        buf: &mut Vec<(PluginIndex, usize, &'a Self)>,
        oneshots: &mut Vec<(PluginIndex, usize)>,
    ) -> Self::Effects {
        let mut effects = Self::Effects::default();

        for (index, plugin) in plugins.iter().enumerate() {
            if plugin.disabled {
                continue;
            }
            let senders = if plugin.metadata.is_world_plugin {
                Self::from_world(world)
            } else {
                Self::from_plugin(plugin)
            };
            for (i, sender) in senders.iter().enumerate() {
                let reaction: &Reaction = sender.as_ref();
                if !matches!(reaction.regex.is_match(line), Ok(true)) {
                    continue;
                }
                buf.push((index, i, sender));
                if !reaction.enabled {
                    continue;
                }
                sender.add_effects(&mut effects);
                if reaction.one_shot {
                    oneshots.push((index, i));
                }
                if !reaction.keep_evaluating {
                    break;
                }
            }
        }

        effects
    }
}

impl ReactionIterable for Alias {
    type Effects = AliasEffects;

    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}

impl ReactionIterable for Trigger {
    type Effects = TriggerEffects;

    fn add_effects(&self, effects: &mut Self::Effects) {
        effects.add_effects(self);
    }
}
