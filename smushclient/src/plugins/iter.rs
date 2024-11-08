use smushclient_plugins::{Alias, CursorVec, Plugin, Sender, Timer, Trigger};

use super::send::SenderAccessError;
use crate::world::World;

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Ord + Sized {
    fn from_plugin(plugin: &Plugin) -> &[Self];
    fn from_plugin_mut(plugin: &mut Plugin) -> &mut CursorVec<Self>;
    fn from_world(world: &World) -> &[Self];
    fn from_world_mut(world: &mut World) -> &mut CursorVec<Self>;
    fn from_either<'a>(plugin: &'a Plugin, world: &'a World) -> &'a [Self] {
        if plugin.metadata.is_world_plugin {
            Self::from_world(world)
        } else {
            Self::from_plugin(plugin)
        }
    }
    fn from_either_mut<'a>(
        plugin: &'a mut Plugin,
        world: &'a mut World,
    ) -> &'a mut CursorVec<Self> {
        if plugin.metadata.is_world_plugin {
            Self::from_world_mut(world)
        } else {
            Self::from_plugin_mut(plugin)
        }
    }
    fn assert_unique_label(
        &self,
        senders: &[Self],
        i: Option<usize>,
    ) -> Result<(), SenderAccessError> {
        let label = self.as_ref().label.as_str();
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
}

macro_rules! impl_send_iterable {
    ($t:ty, $i:ident) => {
        impl SendIterable for $t {
            fn from_plugin(plugin: &Plugin) -> &[Self] {
                &plugin.$i
            }
            fn from_plugin_mut(plugin: &mut Plugin) -> &mut CursorVec<Self> {
                &mut plugin.$i
            }
            fn from_world(plugin: &World) -> &[Self] {
                &plugin.$i
            }
            fn from_world_mut(world: &mut World) -> &mut CursorVec<Self> {
                &mut world.$i
            }
        }
    };
}

impl_send_iterable!(Alias, aliases);
impl_send_iterable!(Timer, timers);
impl_send_iterable!(Trigger, triggers);
