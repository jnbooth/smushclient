use std::hash::Hash;
use std::iter::Enumerate;
use std::{iter, slice, str, vec};

use fancy_regex::{CaptureMatches, Captures};

use super::pad::Pad;
use super::plugin::{Plugin, PluginMetadata};
use crate::send::{Alias, Reaction, SendTarget, Sender, Timer, Trigger};

pub type PluginIndex = usize;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Indexed<T> {
    // Note: this is at the top for Ord-deriving purposes.
    val: T,
    plugin: PluginIndex,
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Indexer<T> {
    inner: Vec<Indexed<T>>,
}

impl<T> Default for Indexer<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Indexer<T> {
    pub const fn new() -> Self {
        Self { inner: Vec::new() }
    }

    pub fn clear(&mut self) {
        self.inner.clear();
    }

    pub fn replace<'a, I>(&mut self, index: PluginIndex, iter: I)
    where
        I: IntoIterator<Item = &'a T>,
        T: 'a + Clone,
    {
        self.inner.retain(|x| x.plugin != index);
        self.extend(index, iter);
    }

    pub fn extend<'a, I>(&mut self, index: PluginIndex, iter: I)
    where
        I: IntoIterator<Item = &'a T>,
        T: 'a + Clone,
    {
        let new_iter = iter.into_iter().map(|content| Indexed {
            plugin: index,
            val: content.to_owned(),
        });
        self.inner.extend(new_iter);
    }

    pub fn sort(&mut self)
    where
        T: Ord,
    {
        self.inner.sort_unstable();
    }

    pub fn add(&mut self, plugin: PluginIndex, val: T)
    where
        T: Ord,
    {
        self.inner.push(Indexed { val, plugin });
        self.sort();
    }

    pub fn iter(&self) -> impl Iterator<Item = (PluginIndex, &T)> {
        self.into_iter()
    }

    pub fn iter_mut(&mut self) -> impl Iterator<Item = (PluginIndex, &mut T)> {
        self.into_iter()
    }

    pub fn matches<'a, 'b: 'a>(
        &'a mut self,
        line: &'b str,
    ) -> impl Iterator<Item = SendMatch<'a, 'b, T>>
    where
        T: AsRef<Reaction> + Clone,
    {
        MatchingIter {
            inner: self.inner.iter().enumerate(),
            line,
            repeating: RepeatingMatch::Empty,
        }
    }

    pub fn find_by_label(&self, label: &str) -> Option<&T>
    where
        T: AsRef<Sender>,
    {
        self.inner
            .iter()
            .map(|item| &item.val)
            .find(|item| item.as_ref().label == label)
    }

    pub fn find_by_label_mut(&mut self, label: &str) -> Option<&mut T>
    where
        T: AsRef<Sender>,
    {
        self.inner
            .iter_mut()
            .map(|item| &mut item.val)
            .find(|item| item.as_ref().label == label)
    }
}

enum RepeatingMatch<'a, 'b, T> {
    Empty,
    Repeat(usize, SendMatch<'a, 'b, T>),
    Captures {
        pos: usize,
        iter: CaptureMatches<'a, 'b>,
        indexed: &'a Indexed<T>,
    },
}

impl<'a, 'b, T: AsRef<Reaction>> Iterator for RepeatingMatch<'a, 'b, T> {
    type Item = SendMatch<'a, 'b, T>;

    fn next(&mut self) -> Option<Self::Item> {
        let next = match self {
            RepeatingMatch::Empty => return None,
            RepeatingMatch::Repeat(0, matched) => Some(matched.clone_uncaptured()),
            RepeatingMatch::Repeat(times, matched) => {
                *times -= 1;
                return Some(matched.clone_uncaptured());
            }
            RepeatingMatch::Captures { iter, pos, indexed } => match iter.next() {
                Some(Ok(captures)) => {
                    let reaction = indexed.val.as_ref();
                    return Some(SendMatch {
                        plugin: indexed.plugin,
                        sender: &indexed.val,
                        pos: *pos,
                        text: &reaction.text,
                        captures: Some(captures),
                    });
                }
                _ => None,
            },
        };
        *self = RepeatingMatch::Empty;
        next
    }
}

struct MatchingIter<'a, 'b, T> {
    inner: Enumerate<slice::Iter<'a, Indexed<T>>>,
    line: &'b str,
    repeating: RepeatingMatch<'a, 'b, T>,
}

impl<'a, 'b, T: AsRef<Reaction>> Iterator for MatchingIter<'a, 'b, T> {
    type Item = SendMatch<'a, 'b, T>;

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(repeated) = self.repeating.next() {
            return Some(repeated);
        }
        for (pos, indexed) in &mut self.inner {
            let reaction = indexed.val.as_ref();
            if !reaction.enabled || !matches!(reaction.regex.is_match(self.line), Ok(true)) {
                continue;
            }
            let count = if reaction.repeats {
                reaction.regex.find_iter(self.line).count()
            } else {
                1
            };
            if reaction.script.is_empty() && !reaction.text.contains('$') {
                let val = SendMatch {
                    plugin: indexed.plugin,
                    sender: &indexed.val,
                    pos,
                    text: &reaction.text,
                    captures: None,
                };
                if count > 1 {
                    self.repeating = RepeatingMatch::Repeat(count - 2, val.clone_uncaptured());
                }
                return Some(val);
            }
            let iter = reaction.regex.captures_iter(self.line);
            self.repeating = RepeatingMatch::Captures { iter, pos, indexed };
            if let Some(repeated) = self.repeating.next() {
                return Some(repeated);
            }
        }
        None
    }
}

#[allow(clippy::type_complexity)]
impl<T> IntoIterator for Indexer<T> {
    type Item = (PluginIndex, T);
    type IntoIter = iter::Map<vec::IntoIter<Indexed<T>>, fn(Indexed<T>) -> (PluginIndex, T)>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.into_iter().map(|i| (i.plugin, i.val))
    }
}

#[allow(clippy::type_complexity)]
impl<'a, T> IntoIterator for &'a Indexer<T> {
    type Item = (PluginIndex, &'a T);
    type IntoIter = iter::Map<slice::Iter<'a, Indexed<T>>, fn(&Indexed<T>) -> (PluginIndex, &T)>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter().map(|i| (i.plugin, &i.val))
    }
}

#[allow(clippy::type_complexity)]
impl<'a, T> IntoIterator for &'a mut Indexer<T> {
    type Item = (PluginIndex, &'a mut T);
    type IntoIter =
        iter::Map<slice::IterMut<'a, Indexed<T>>, fn(&mut Indexed<T>) -> (PluginIndex, &mut T)>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter_mut().map(|i| (i.plugin, &mut i.val))
    }
}

#[derive(Debug)]
pub struct SendMatch<'a, 'b, T> {
    pub plugin: PluginIndex,
    pub pos: usize,
    pub sender: &'a T,
    text: &'a str,
    captures: Option<Captures<'b>>,
}

impl<'a, 'b, T> SendMatch<'a, 'b, T> {
    pub const fn is_empty(&self) -> bool {
        self.text.is_empty()
    }

    pub fn text<'c, 'd>(&self, buf: &'c mut String) -> &'d str
    where
        'a: 'd,
        'c: 'd,
    {
        match &self.captures {
            Some(ref captures) => {
                buf.clear();
                captures.expand(self.text, buf);
                buf.as_str()
            }
            None => self.text,
        }
    }

    pub fn wildcards(&self) -> Vec<&'b str> {
        let Some(ref captures) = &self.captures else {
            return Vec::new();
        };
        let mut wildcards = Vec::with_capacity(captures.len() - 1);
        let mut iter = captures.iter();
        iter.next(); // skip the all-encompassing capture
        for capture in iter.flatten() {
            wildcards.push(capture.as_str());
        }
        wildcards
    }

    fn clone_uncaptured(&self) -> Self {
        Self {
            plugin: self.plugin,
            pos: self.pos,
            sender: self.sender,
            text: self.text,
            captures: None,
        }
    }
}

impl<'a, 'b, T: Sendable> SendMatch<'a, 'b, T> {
    pub fn has_send(&self) -> bool {
        let sender = self.sender.as_ref();
        let send_to = sender.send_to;
        !send_to.ignore_empty()
            || !self.text.is_empty()
            || !sender.script.is_empty()
            || (send_to == SendTarget::Variable && !sender.variable.is_empty())
    }

    pub fn pad(&'a self, plugins: &'a [Plugin]) -> Option<Pad<'a>> {
        let sender = self.sender.as_ref();
        if sender.send_to.is_notepad() {
            Some(self.sender.pad(&plugins[self.plugin].metadata))
        } else {
            None
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Senders {
    aliases: Indexer<Alias>,
    timers: Indexer<Timer>,
    triggers: Indexer<Trigger>,
}

impl Senders {
    pub const fn new() -> Self {
        Self {
            aliases: Indexer::new(),
            timers: Indexer::new(),
            triggers: Indexer::new(),
        }
    }

    pub fn clear(&mut self) {
        self.triggers.clear();
        self.aliases.clear();
        self.timers.clear();
    }

    pub fn extend(&mut self, i: PluginIndex, plugin: &Plugin) {
        self.triggers.extend(i, &plugin.triggers);
        self.aliases.extend(i, &plugin.aliases);
        self.timers.extend(i, &plugin.timers);
    }

    pub fn sort(&mut self) {
        self.triggers.sort();
        self.aliases.sort();
        self.timers.sort();
    }

    pub fn replace_all<T: Sendable + Ord + Clone>(&mut self, i: PluginIndex, vals: &[T]) {
        let indexer = T::indexer_mut(self);
        indexer.replace(i, vals);
        indexer.sort();
    }

    pub fn matches<'a, 'b: 'a, T: Sendable + AsRef<Reaction> + Clone>(
        &'a mut self,
        line: &'b str,
    ) -> impl Iterator<Item = SendMatch<'a, 'b, T>> {
        T::indexer_mut(self).matches(line)
    }

    // Given a sorted and deduplicated list of vector positions, deletes the elements corresponding
    // to those positions.
    pub fn delete_all<T: Sendable>(&mut self, positions: &[usize]) {
        let indexer = &mut T::indexer_mut(self).inner;
        for &pos in positions.iter().rev() {
            indexer.remove(pos);
        }
    }

    pub fn add<T: Sendable>(&mut self, plugin: PluginIndex, val: T) {
        T::indexer_mut(self).add(plugin, val);
    }

    pub fn replace<T: Sendable>(&mut self, plugin: PluginIndex, val: T) {
        let indexer = T::indexer_mut(self);
        let sender = val.as_ref();
        indexer.inner.retain(|indexed| {
            indexed.plugin != plugin || {
                let other = indexed.val.as_ref();
                sender.label != other.label || sender.group != other.group
            }
        });
        indexer.add(plugin, val);
    }
}

pub trait Sendable: 'static + Sized + Ord + AsRef<Sender> + AsMut<Sender> {
    fn indexer(indices: &Senders) -> &Indexer<Self>;
    fn indexer_mut(indices: &mut Senders) -> &mut Indexer<Self>;
    fn pad<'a>(&'a self, metadata: &'a PluginMetadata) -> Pad<'a>;
}
impl Sendable for Alias {
    fn indexer(indices: &Senders) -> &Indexer<Self> {
        &indices.aliases
    }
    fn indexer_mut(indices: &mut Senders) -> &mut Indexer<Self> {
        &mut indices.aliases
    }

    fn pad<'a>(&'a self, metadata: &'a PluginMetadata) -> Pad<'a> {
        Pad::Alias {
            plugin: &metadata.name,
            label: if self.label.is_empty() {
                &self.pattern
            } else {
                &self.label
            },
        }
    }
}
impl Sendable for Timer {
    fn indexer(indices: &Senders) -> &Indexer<Self> {
        &indices.timers
    }
    fn indexer_mut(indices: &mut Senders) -> &mut Indexer<Self> {
        &mut indices.timers
    }

    fn pad<'a>(&'a self, metadata: &'a PluginMetadata) -> Pad<'a> {
        Pad::Timer {
            plugin: &metadata.name,
            occurrence: self.occurrence,
        }
    }
}
impl Sendable for Trigger {
    fn indexer(indices: &Senders) -> &Indexer<Self> {
        &indices.triggers
    }
    fn indexer_mut(indices: &mut Senders) -> &mut Indexer<Self> {
        &mut indices.triggers
    }

    fn pad<'a>(&'a self, metadata: &'a PluginMetadata) -> Pad<'a> {
        Pad::Trigger {
            plugin: &metadata.name,
            label: if self.label.is_empty() {
                &self.pattern
            } else {
                &self.label
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    mod indexer {
        use super::*;
        #[test]
        fn test_clear() {
            let mut indexer = Indexer::new();
            indexer.extend(0, &[0, 1, 2, 3, 4]);
            indexer.clear();
            assert_eq!(indexer, Indexer::new());
        }

        #[test]
        fn test_replace() {
            let a: Vec<u8> = (0..5).collect();
            let b: Vec<u8> = (7..10).collect();
            let c: Vec<u8> = (9..13).collect();
            let mut indexer1 = Indexer::new();
            indexer1.replace(0, &a);
            indexer1.replace(1, &b);
            indexer1.replace(0, &c);
            let mut indexer2 = Indexer::new();
            indexer2.replace(1, &b);
            indexer2.replace(0, &c);
            assert_eq!(indexer1, indexer2);
        }

        #[test]
        fn test_sort() {
            let mut indexer = Indexer::new();
            indexer.extend(0, &[1, 5, 3, 2, 6]);
            indexer.extend(1, &[5, 3, 1, 2, 1]);
            indexer.sort();
            let unsorted: Vec<_> = indexer.iter().map(|(_, x)| *x).collect();
            let mut sorted = unsorted.clone();
            sorted.sort_unstable();
            assert_eq!(sorted, unsorted);
        }
    }

    mod senders {

        use super::*;

        fn basic_plugin() -> Plugin {
            Plugin {
                triggers: vec![Trigger::default()],
                aliases: vec![Alias::default()],
                timers: vec![Timer::default()],
                metadata: PluginMetadata::default(),
                script: String::default(),
            }
        }

        #[test]
        fn test_clear() {
            let mut senders = Senders::new();
            senders.extend(2, &basic_plugin());
            senders.clear();
            assert_eq!(
                (
                    senders.aliases.inner.len(),
                    senders.timers.inner.len(),
                    senders.triggers.inner.len()
                ),
                (0, 0, 0)
            );
        }

        fn vals<T>(indexer: Indexer<T>) -> Vec<T> {
            indexer.into_iter().map(|(_, x)| x).collect()
        }

        #[test]
        fn test_extend() {
            let mut senders = Senders::new();
            let plugin = basic_plugin();
            senders.extend(2, &plugin);
            senders.sort();
            assert_eq!(
                (
                    vals(senders.aliases),
                    vals(senders.timers),
                    vals(senders.triggers)
                ),
                (plugin.aliases, plugin.timers, plugin.triggers)
            );
        }

        #[test]
        fn test_delete_all() {
            let mut senders = Senders::new();
            let mut aliases: Vec<_> = (0..11)
                .map(|i| {
                    let mut alias = Alias::default();
                    alias.send.one_shot = i % 3 == 0;
                    alias.label = i.to_string();
                    alias
                })
                .collect();
            senders.aliases.extend(0, &aliases);
            let mut delete_oneshots = Vec::new();
            for send in senders.matches::<Alias>("") {
                if send.sender.one_shot {
                    delete_oneshots.push(send.pos);
                }
            }
            senders.delete_all::<Alias>(&delete_oneshots);
            aliases.retain(|x| !x.one_shot);
            assert_eq!(vals(senders.aliases), aliases);
        }

        struct Opts<'a> {
            pattern: &'a str,
            is_regex: bool,
            repeats: bool,
            text: &'a str,
            line: &'a str,
            expect: &'a [&'a str],
        }

        #[allow(clippy::needless_pass_by_value)]
        fn test_match(opts: Opts) {
            let mut alias = Alias::default();
            alias.reaction.regex = Reaction::make_regex(opts.pattern, opts.is_regex).unwrap();
            alias.repeats = opts.repeats;
            alias.text = opts.text.to_owned();
            let mut senders = Senders::new();
            senders.aliases.add(1, alias);
            let outputs: Vec<_> = senders
                .matches::<Alias>(opts.line)
                .map(|send| send.text.to_owned())
                .collect();
            let expect: Vec<_> = opts.expect.iter().map(ToOwned::to_owned).collect();
            assert_eq!(outputs, expect);
        }

        #[test]
        fn matches_no_match() {
            test_match(Opts {
                pattern: "a*e",
                is_regex: true,
                repeats: false,
                text: "abcdeae",
                line: "b",
                expect: &[],
            });
        }

        #[test]
        fn matches_no_regex() {
            test_match(Opts {
                pattern: "a*e",
                is_regex: false,
                repeats: false,
                text: "0",
                line: "abcdeae",
                expect: &["0"],
            });
        }

        #[test]
        fn matches_regex() {
            test_match(Opts {
                pattern: "a.*e",
                is_regex: true,
                repeats: false,
                text: "0",
                line: "0abcdefgae",
                expect: &["0"],
            });
        }

        #[test]
        fn matches_captures() {
            test_match(Opts {
                pattern: "^(?>tiny |small |medium |large |huge |enormous )(.+) stencil(.+)$",
                is_regex: true,
                repeats: false,
                text: "a $2 $3 \\$1",
                line: "small green stencil!",
                expect: &["a !  \\green"],
            });
        }

        #[test]
        fn matches_repeat() {
            test_match(Opts {
                pattern: "a([^ ]*)e",
                is_regex: true,
                repeats: true,
                text: "f$2$1",
                line: "abcde ade ae",
                expect: &["fbcd", "fd", "f"],
            });
        }
    }
}
