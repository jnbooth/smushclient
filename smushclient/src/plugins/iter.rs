use std::marker::PhantomData;
use std::num::NonZero;
use std::slice;

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

pub(crate) struct AllSendersIter<'a, T> {
    inner: slice::Iter<'a, Plugin>,
    world: &'a World,
    marker: PhantomData<T>,
}

impl<'a, T> AllSendersIter<'a, T> {
    pub fn new(plugins: &'a [Plugin], world: &'a World) -> Self {
        Self {
            inner: plugins.iter(),
            world,
            marker: PhantomData,
        }
    }
}

impl<'a, T> Iterator for AllSendersIter<'a, T>
where
    T: SendIterable,
{
    type Item = &'a CursorVec<T>;

    fn next(&mut self) -> Option<Self::Item> {
        let plugin = self.inner.next()?;
        Some(T::from_either(plugin, self.world))
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let exact = self.len();
        (exact, Some(exact))
    }

    fn count(self) -> usize {
        self.len()
    }

    fn last(mut self) -> Option<Self::Item> {
        self.next_back()
    }

    fn nth(&mut self, n: usize) -> Option<Self::Item> {
        let plugin = self.inner.nth(n)?;
        Some(T::from_either(plugin, self.world))
    }
}

impl<T> ExactSizeIterator for AllSendersIter<'_, T>
where
    T: SendIterable,
{
    fn len(&self) -> usize {
        self.inner.len()
    }
}

impl<T> DoubleEndedIterator for AllSendersIter<'_, T>
where
    T: SendIterable,
{
    fn next_back(&mut self) -> Option<Self::Item> {
        let plugin = self.inner.next_back()?;
        Some(T::from_either(plugin, self.world))
    }

    fn nth_back(&mut self, n: usize) -> Option<Self::Item> {
        let plugin = self.inner.nth_back(n)?;
        Some(T::from_either(plugin, self.world))
    }
}
