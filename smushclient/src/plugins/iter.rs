use smushclient_plugins::{Alias, CursorVec, Plugin, Sender, Timer, Trigger, XmlError};

use crate::world::World;

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Ord + Sized {
    fn from_xml_str(s: &str) -> Result<Vec<Self>, XmlError>;
    fn to_xml_string<'a, I: IntoIterator<Item = &'a Self>>(iter: I) -> Result<String, XmlError>
    where
        Self: 'a;
    fn from_plugin(plugin: &Plugin) -> &CursorVec<Self>;
    fn from_plugin_mut(plugin: &mut Plugin) -> &mut CursorVec<Self>;
    fn from_world(world: &World) -> &CursorVec<Self>;
    fn from_world_mut(world: &mut World) -> &mut CursorVec<Self>;
    fn from_either<'a>(plugin: &'a Plugin, world: &'a World) -> &'a CursorVec<Self> {
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
    ($t:ty, $i:ident) => {
        impl SendIterable for $t {
            fn from_xml_str(s: &str) -> Result<Vec<Self>, XmlError> {
                Self::from_xml_str(s)
            }
            fn to_xml_string<'a, I: IntoIterator<Item = &'a Self>>(
                iter: I,
            ) -> Result<String, XmlError> {
                Self::to_xml_string(iter)
            }
            fn from_plugin(plugin: &Plugin) -> &CursorVec<Self> {
                &plugin.$i
            }
            fn from_plugin_mut(plugin: &mut Plugin) -> &mut CursorVec<Self> {
                &mut plugin.$i
            }
            fn from_world(plugin: &World) -> &CursorVec<Self> {
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
