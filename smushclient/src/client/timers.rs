use std::ops::{Deref, DerefMut};

use super::reuse_vec::ReuseVec;
use super::SmushClient;
use crate::handler::TimerHandler;
use smushclient_plugins::{Occurrence, PluginIndex, Timer};

pub trait TimerConstructible {
    fn construct(index: PluginIndex, timer: &Timer) -> Self;
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct SendTimer<T> {
    pub plugin: PluginIndex,
    pub id: u16,
    pub milliseconds: u32,
    pub one_shot: bool,
    pub timer: T,
}

impl<T: TimerConstructible> SendTimer<T> {
    pub fn new(plugin: PluginIndex, timer: &Timer) -> Option<Self> {
        if !timer.enabled {
            return None;
        }
        let Occurrence::Interval(duration) = timer.occurrence else {
            return None;
        };
        Some(Self {
            plugin,
            id: timer.id,
            milliseconds: u32::try_from(duration.as_millis()).unwrap_or(u32::MAX),
            one_shot: timer.one_shot,
            timer: T::construct(plugin, timer),
        })
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Timers<T> {
    inner: ReuseVec<SendTimer<T>>,
}

impl<T> Default for Timers<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Deref for Timers<T> {
    type Target = ReuseVec<SendTimer<T>>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<T> DerefMut for Timers<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

impl<T> Timers<T> {
    pub const fn new() -> Self {
        Self {
            inner: ReuseVec::new(),
        }
    }
}

impl<T: TimerConstructible> Timers<T> {
    pub fn start<H: TimerHandler<T>>(
        &mut self,
        index: PluginIndex,
        timer: &Timer,
        handler: &mut H,
    ) {
        let Some(send_timer) = SendTimer::new(index, timer) else {
            return;
        };
        let milliseconds = send_timer.milliseconds;
        let id = self.insert(send_timer);
        handler.start_timer(id, milliseconds);
    }

    pub fn finish<H: TimerHandler<T>>(
        &mut self,
        id: usize,
        client: &mut SmushClient,
        handler: &mut H,
    ) -> bool {
        let Some(send_timer) = self.inner.get(id) else {
            return true;
        };
        handler.send_timer(&send_timer.timer);
        if !send_timer.one_shot {
            return false;
        }
        let timers = client.senders_mut::<Timer>(send_timer.plugin);
        if let Some(i) = timers.iter().position(|timer| timer.id == send_timer.id) {
            timers.remove(i);
        }
        self.inner.remove(id);
        true
    }
}
