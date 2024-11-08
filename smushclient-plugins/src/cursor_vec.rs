use std::cmp::Ordering;
use std::ops::{Deref, DerefMut};
use std::{slice, vec};

use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CursorVec<T> {
    inner: Vec<T>,
    cursor: usize,
    removals: Vec<usize>,
    additions: Vec<T>,
}

impl<T> CursorVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: Vec::new(),
            cursor: 0,
            removals: Vec::new(),
            additions: Vec::new(),
        }
    }
}

impl<T: Ord> CursorVec<T> {
    pub fn begin(&mut self) {
        self.apply_pending(true);
        self.cursor = 0;
    }

    pub fn end(&mut self) {
        self.cursor = self.inner.len();
        self.apply_pending(true);
    }

    pub fn done(&self) -> bool {
        self.cursor >= self.inner.len()
    }

    #[allow(clippy::should_implement_trait)]
    pub fn next(&mut self) -> Option<&mut T> {
        if self.done() {
            self.apply_pending(true);
            return None;
        }
        self.apply_pending(false);
        self.cursor += 1;
        self.inner.get_mut(self.cursor - 1)
    }

    pub fn append(&mut self, other: &mut Vec<T>) {
        if self.done() {
            self.inner.append(other);
            self.inner.sort_unstable();
        } else {
            self.additions.append(other);
        }
    }

    fn apply_pending(&mut self, done: bool) {
        for &i in self.removals.iter().rev() {
            self.inner.remove(i);
            if !done && i <= self.cursor {
                self.cursor -= 1;
            }
        }
        for item in self.additions.drain(..) {
            if Self::do_insert(&mut self.inner, item) < self.cursor && !done {
                self.cursor += 1;
            }
        }
        self.removals.clear();
    }

    pub fn insert(&mut self, item: T) {
        if self.done() {
            Self::do_insert(&mut self.inner, item);
        } else {
            self.additions.push(item);
        }
    }

    fn do_insert(inner: &mut Vec<T>, item: T) -> usize {
        let pos = match inner.binary_search(&item) {
            Ok(pos) => {
                inner[pos] = item;
                return pos;
            }
            Err(pos) => pos,
        };
        inner.insert(pos, item);
        pos
    }

    pub fn replace(&mut self, i: usize, item: T) {
        let pos = match self.inner.binary_search(&item) {
            Ok(pos) | Err(pos) => pos,
        };
        if pos == i {
            self.inner[i] = item;
            return;
        }
        if !self.done() {
            self.removals.push(i);
            self.additions.push(item);
            return;
        }
        self.inner[i] = item;
        match i.cmp(&pos) {
            Ordering::Less => self.inner[i..=pos].rotate_left(1),
            Ordering::Equal => (),
            Ordering::Greater => self.inner[pos..=i].rotate_right(1),
        }
    }

    pub fn remove(&mut self, i: usize) {
        if i > self.cursor || self.done() {
            self.inner.remove(i);
        } else {
            self.removals.push(i);
        }
    }

    pub fn remove_current(&mut self) {
        self.removals.push(self.cursor);
    }

    pub fn position<P: FnMut(&T) -> bool>(&mut self, mut pred: P) -> Option<usize> {
        let (i, _) = self
            .inner
            .iter()
            .enumerate()
            .find(|(i, item)| pred(item) && !self.removals.contains(i))?;
        Some(i)
    }

    pub fn retain<P: FnMut(&T) -> bool>(&mut self, mut pred: P) -> usize {
        let mut removed = 0;
        for i in (0..self.inner.len()).rev() {
            if !pred(&self.inner[i]) {
                self.remove(i);
                removed += 1;
            }
        }
        removed
    }
}

impl<T> From<Vec<T>> for CursorVec<T> {
    fn from(value: Vec<T>) -> Self {
        Self {
            inner: value,
            ..Default::default()
        }
    }
}

impl<T> From<CursorVec<T>> for Vec<T> {
    fn from(value: CursorVec<T>) -> Self {
        value.inner
    }
}

impl<T> Default for CursorVec<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Deref for CursorVec<T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<T> DerefMut for CursorVec<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

impl<T> FromIterator<T> for CursorVec<T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> Self {
        Vec::from_iter(iter).into()
    }
}

impl<T> IntoIterator for CursorVec<T> {
    type Item = T;

    type IntoIter = vec::IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.into_iter()
    }
}

impl<'a, T> IntoIterator for &'a CursorVec<T> {
    type Item = &'a T;

    type IntoIter = slice::Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter()
    }
}

impl<'a, T> IntoIterator for &'a mut CursorVec<T> {
    type Item = &'a mut T;

    type IntoIter = slice::IterMut<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter_mut()
    }
}

impl<T: Serialize> Serialize for CursorVec<T> {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        self.inner.serialize(serializer)
    }
}

impl<'de, T: Deserialize<'de>> Deserialize<'de> for CursorVec<T> {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        Vec::deserialize(deserializer).map(Self::from)
    }
}
