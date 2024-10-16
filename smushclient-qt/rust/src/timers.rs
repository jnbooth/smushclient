use cxx_qt_lib::QString;
use smushclient_plugins::{Occurrence, PluginIndex, Timer, Uuid};

use crate::ffi;

impl Clone for ffi::SendTimer {
    fn clone(&self) -> Self {
        Self {
            active_closed: self.active_closed,
            label: self.label.clone(),
            plugin: self.plugin,
            script: self.script.clone(),
            target: self.target,
            text: self.text.clone(),
        }
    }
}

#[derive(Clone)]
pub struct SendTimer {
    pub id: Uuid,
    pub milliseconds: u32,
    pub one_shot: bool,
    pub qt: ffi::SendTimer,
}

impl SendTimer {
    pub fn new(plugin: PluginIndex, timer: &Timer) -> Option<Self> {
        let Occurrence::Interval(duration) = timer.occurrence else {
            return None;
        };
        let qt = ffi::SendTimer {
            active_closed: timer.active_closed,
            label: timer.label.clone(),
            plugin,
            script: timer.script.clone(),
            target: timer.send_to.into(),
            text: QString::from(&timer.text),
        };
        Some(Self {
            id: timer.id,
            milliseconds: u32::try_from(duration.as_millis()).unwrap_or(u32::MAX),
            one_shot: timer.one_shot,
            qt,
        })
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ReuseVec<T> {
    inner: Vec<T>,
    holes: Vec<usize>,
}

impl<T> Default for ReuseVec<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> ReuseVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: Vec::new(),
            holes: Vec::new(),
        }
    }

    pub fn get(&self, pos: usize) -> Option<&T> {
        self.inner.get(pos)
    }

    pub fn insert(&mut self, timer: T) -> usize {
        let Some(hole) = self.holes.pop() else {
            self.inner.push(timer);
            return self.inner.len() - 1;
        };
        self.inner[hole] = timer;
        hole
    }

    pub fn remove(&mut self, index: usize) {
        if index >= self.holes.len() {
            return;
        }
        self.holes.push(index);
    }
}
