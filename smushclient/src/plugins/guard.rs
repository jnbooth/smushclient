use smushclient_plugins::{Plugin, PluginIndex};

use crate::world::World;

use super::iter::SendIterable;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct SenderGuard {
    defer: bool,
    needs_sort: Vec<bool>,
    removals: Vec<Vec<usize>>,
}

impl Default for SenderGuard {
    fn default() -> Self {
        Self::new()
    }
}

impl SenderGuard {
    pub const fn new() -> Self {
        Self {
            defer: false,
            needs_sort: Vec::new(),
            removals: Vec::new(),
        }
    }

    pub fn set_plugin_count(&mut self, count: usize) {
        if count <= self.needs_sort.len() {
            return;
        }
        self.needs_sort.resize(count, false);
        self.removals.resize_with(count, Default::default);
    }
}

impl SenderGuard {
    pub fn defer(&mut self) {
        self.defer = true;
    }

    pub fn add<'a, T: Ord>(
        &mut self,
        plugin: PluginIndex,
        senders: &'a mut Vec<T>,
        sender: T,
    ) -> (usize, &'a T) {
        let pos = if self.defer {
            self.needs_sort[plugin] = true;
            senders.push(sender);
            senders.len() - 1
        } else {
            let pos = match senders.binary_search(&sender) {
                Ok(pos) | Err(pos) => pos,
            };
            senders.insert(pos, sender);
            pos
        };
        (pos, &senders[pos])
    }

    pub fn defer_remove(&mut self, plugin: PluginIndex, pos: usize) {
        self.removals[plugin].push(pos);
    }

    pub fn remove<T>(
        &mut self,
        plugin: PluginIndex,
        senders: &mut Vec<T>,
        pos: usize,
    ) -> Option<T> {
        if plugin >= senders.len() {
            return None;
        }
        if self.defer {
            self.removals[plugin].push(pos);
            return None;
        }
        Some(senders.remove(pos))
    }

    pub fn remove_all<T, P: FnMut(&T) -> bool>(
        &mut self,
        plugin: PluginIndex,
        senders: &mut Vec<T>,
        mut pred: P,
    ) -> usize {
        if !self.defer {
            let len = senders.len();
            senders.retain(|sender| !pred(sender));
            return len - senders.len();
        }
        let removals = &mut self.removals[plugin];
        let len = removals.len();
        removals.extend(
            senders
                .iter()
                .enumerate()
                .filter(|(_, sender)| pred(sender))
                .map(|(pos, _)| pos),
        );
        removals.len() - len
    }

    pub fn finalize<T: SendIterable>(&mut self, plugins: &mut [Plugin], world: &mut World) {
        for ((plugin, needs_sort), removals) in plugins
            .iter_mut()
            .zip(self.needs_sort.iter_mut())
            .zip(self.removals.iter_mut())
        {
            let senders = T::from_either_mut(plugin, world);
            if *needs_sort {
                senders.sort_unstable();
                *needs_sort = false;
            }
            if removals.is_empty() {
                continue;
            }
            removals.sort_unstable();
            removals.dedup();
            for &removal in removals.iter().rev() {
                senders.remove(removal);
            }
            removals.clear();
        }
        self.defer = false;
    }
}
