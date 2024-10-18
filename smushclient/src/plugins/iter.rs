use smushclient_plugins::{Alias, Plugin, PluginIndex, Sender, Timer, Trigger};

use crate::World;

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Ord + Sized {
    fn from_plugin(plugin: &Plugin) -> &[Self];
    fn from_plugin_mut(plugin: &mut Plugin) -> &mut Vec<Self>;
    fn from_world(world: &World) -> &[Self];
    fn from_world_mut(world: &mut World) -> &mut Vec<Self>;

    fn iter<'a>(
        plugins: &'a [Plugin],
        world: &'a World,
    ) -> impl Iterator<Item = (PluginIndex, &'a Self)>
    where
        Self: 'a,
    {
        plugins.iter().enumerate().flat_map(|(i, plugin)| {
            if plugin.metadata.is_world_plugin {
                Self::from_world(world)
            } else {
                Self::from_plugin(plugin)
            }
            .iter()
            .map(move |sender| (i, sender))
        })
    }

    fn iter_mut<'a>(
        plugins: &'a mut [Plugin],
        world: &'a mut World,
    ) -> impl Iterator<Item = (PluginIndex, &'a mut Self)>
    where
        Self: 'a,
    {
        let mut world = Some(world);
        plugins.iter_mut().enumerate().flat_map(move |(i, plugin)| {
            if plugin.metadata.is_world_plugin {
                Self::from_world_mut(world.take().expect("more than one world plugin"))
            } else {
                Self::from_plugin_mut(plugin)
            }
            .iter_mut()
            .map(move |sender| (i, sender))
        })
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
        }
    };
}

impl_send_iterable!(Alias, aliases);
impl_send_iterable!(Timer, timers);
impl_send_iterable!(Trigger, triggers);
