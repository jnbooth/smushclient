use std::cmp::Ordering;
use std::hash::{Hash, Hasher};
use std::ops::{Deref, DerefMut};
use std::time::Duration;

use crate::client::SmushClient;
use chrono::NaiveTime;
use smushclient_plugins::{PluginIndex, Timer};

pub trait TimerConstructible {
    fn construct(index: PluginIndex, timer: &Timer) -> Self;
}

#[derive(Copy, Clone, Debug, Default)]
pub struct SendTimer<T> {
    pub plugin: PluginIndex,
    pub id: u16,
    pub one_shot: bool,
    pub timer: T,
}

impl<T> PartialEq for SendTimer<T> {
    fn eq(&self, other: &Self) -> bool {
        self.id.eq(&other.id)
    }
}

impl<T> Eq for SendTimer<T> {}

impl<T> PartialOrd for SendTimer<T> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<T> Ord for SendTimer<T> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.id.cmp(&other.id)
    }
}

impl<T> Hash for SendTimer<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.id.hash(state);
    }
}

impl<T: TimerConstructible> SendTimer<T> {
    pub fn new(plugin: PluginIndex, timer: &Timer) -> Self {
        Self {
            plugin,
            id: timer.id,
            one_shot: timer.one_shot,
            timer: T::construct(plugin, timer),
        }
    }

    pub fn remove(&self, client: &mut SmushClient) -> Option<usize> {
        let timers = client.senders_mut::<Timer>(self.plugin);
        let i = timers.iter().position(|timer| timer.id == self.id)?;
        timers.remove(i);
        Some(i)
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct RecurringTimer<T> {
    inner: SendTimer<T>,
    pub milliseconds: u32,
}

impl<T> RecurringTimer<T> {
    pub fn new(timer: SendTimer<T>, duration: Duration) -> Self {
        Self {
            inner: timer,
            milliseconds: u32::try_from(duration.as_millis()).unwrap_or(u32::MAX),
        }
    }
}

impl<T> Deref for RecurringTimer<T> {
    type Target = SendTimer<T>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}
impl<T> DerefMut for RecurringTimer<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

#[derive(Copy, Clone, Debug, Default)]
pub struct ScheduledTimer<T> {
    pub time: NaiveTime,
    inner: SendTimer<T>,
}

impl<T> ScheduledTimer<T> {
    pub fn new(timer: SendTimer<T>, time: NaiveTime) -> Self {
        Self { time, inner: timer }
    }
}

impl<T> PartialEq for ScheduledTimer<T> {
    fn eq(&self, other: &Self) -> bool {
        self.time == other.time && self.inner == other.inner
    }
}

impl<T> Eq for ScheduledTimer<T> {}

impl<T> PartialOrd for ScheduledTimer<T> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<T> Ord for ScheduledTimer<T> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.time
            .cmp(&other.time)
            .then(self.inner.cmp(&other.inner))
    }
}

impl<T> Hash for ScheduledTimer<T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.time.hash(state);
        self.inner.hash(state);
    }
}

impl<T> Deref for ScheduledTimer<T> {
    type Target = SendTimer<T>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}
impl<T> DerefMut for ScheduledTimer<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}
