use smushclient_plugins::{Alias, Plugin, PluginIndex, Reaction, Sender, Timer, Trigger};

use crate::plugins::effects::{AliasEffects, TriggerEffects};
use crate::World;

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Ord + Sized {
    fn from_plugin(plugin: &Plugin) -> &[Self];
    fn from_plugin_mut(plugin: &mut Plugin) -> &mut Vec<Self>;
    fn from_world(world: &World) -> &[Self];
    fn from_world_mut(world: &mut World) -> &mut Vec<Self>;
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
        }
    };
}

impl_send_iterable!(Alias, aliases);
impl_send_iterable!(Timer, timers);
impl_send_iterable!(Trigger, triggers);

pub trait ReactionIterable: SendIterable + AsRef<Reaction> + AsMut<Reaction> {
    type Effects: Default;

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
            buf.clear();
            let senders = if plugin.metadata.is_world_plugin {
                Self::from_world(world)
            } else {
                Self::from_plugin(plugin)
            };
            for (i, sender) in senders.iter().enumerate() {
                let reaction: &Reaction = sender.as_ref();
                if !reaction.enabled || !matches!(reaction.regex.is_match(line), Ok(true)) {
                    continue;
                }
                if reaction.one_shot {
                    oneshots.push((index, i));
                }
                sender.add_effects(&mut effects);
                buf.push((index, i, sender));
                if !reaction.keep_evaluating {
                    return effects;
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
