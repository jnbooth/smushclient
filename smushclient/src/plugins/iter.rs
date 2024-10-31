use smushclient_plugins::{Alias, Plugin, Sender, Timer, Trigger};

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

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Senders<T> {
    aliases: T,
    timers: T,
    triggers: T,
}

impl<T> Senders<T> {
    pub const fn new(aliases: T, timers: T, triggers: T) -> Self {
        Self {
            aliases,
            timers,
            triggers,
        }
    }
}

impl<T> Senders<T> {
    pub fn get<S: SendIterable>(&self) -> &T {
        S::access_senders(self)
    }

    pub fn get_mut<S: SendIterable>(&mut self) -> &mut T {
        S::access_senders_mut(self)
    }

    pub fn set<S: SendIterable>(&mut self, value: T) {
        *self.get_mut::<S>() = value;
    }
}

pub trait SendIterable: AsRef<Sender> + AsMut<Sender> + Ord + Sized {
    fn access_senders<U>(senders: &Senders<U>) -> &U;
    fn access_senders_mut<U>(senders: &mut Senders<U>) -> &mut U;
    fn from_plugin(plugin: &Plugin) -> &[Self];
    fn from_plugin_mut(plugin: &mut Plugin) -> &mut Vec<Self>;
    fn from_world(world: &World) -> &[Self];
    fn from_world_mut(world: &mut World) -> &mut Vec<Self>;
    fn from_either<'a>(plugin: &'a Plugin, world: &'a World) -> &'a [Self] {
        if plugin.metadata.is_world_plugin {
            Self::from_world(world)
        } else {
            Self::from_plugin(plugin)
        }
    }
    fn from_either_mut<'a>(plugin: &'a mut Plugin, world: &'a mut World) -> &'a mut Vec<Self> {
        if plugin.metadata.is_world_plugin {
            Self::from_world_mut(world)
        } else {
            Self::from_plugin_mut(plugin)
        }
    }
}

macro_rules! impl_send_iterable {
    ($t:ty, $i:ident) => {
        impl SendIterable for $t {
            fn access_senders<U>(senders: &Senders<U>) -> &U {
                &senders.$i
            }
            fn access_senders_mut<U>(senders: &mut Senders<U>) -> &mut U {
                &mut senders.$i
            }
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
