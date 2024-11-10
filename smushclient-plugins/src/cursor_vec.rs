use std::cmp::Ordering;
use std::ops::{Deref, DerefMut};
use std::{slice, vec};

use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CursorVec<T> {
    inner: Vec<T>,
    cursor: usize,
    removals: Vec<usize>,
    additions_index: usize,
    evaluating: bool,
}

impl<T> CursorVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: Vec::new(),
            cursor: 0,
            removals: Vec::new(),
            additions_index: 0,
            evaluating: false,
        }
    }
}

impl<T: Ord> CursorVec<T> {
    pub fn begin(&mut self) {
        self.apply_pending(true);
        self.evaluating = true;
        self.additions_index = self.inner.len();
        self.cursor = 0;
    }

    pub fn end(&mut self) {
        self.apply_pending(true);
        self.evaluating = false;
    }

    #[allow(clippy::should_implement_trait)]
    pub fn next(&mut self) -> Option<&mut T> {
        if self.cursor >= self.inner.len() {
            self.end();
            return None;
        }
        self.apply_pending(false);
        self.cursor += 1;
        self.additions_index = self.inner.len();
        self.inner.get_mut(self.cursor - 1)
    }

    pub fn append(&mut self, other: &mut Vec<T>) {
        self.inner.append(other);
    }

    fn apply_pending(&mut self, done: bool) {
        for &i in self.removals.iter().rev() {
            self.inner.remove(i);
            self.additions_index -= 1;
            if !done && i <= self.cursor {
                self.cursor -= 1;
            }
        }
        self.removals.clear();
        if !self.evaluating {
            return;
        }
        while self.additions_index < self.inner.len() {
            let Some(item) = self.inner.pop() else {
                break;
            };
            let Err(pos) = self.do_insert(item) else {
                continue;
            };
            self.additions_index += 1;
            if !done && pos < self.cursor {
                self.cursor += 1;
            }
        }
    }

    pub fn insert(&mut self, item: T) -> &T {
        let pos = if self.evaluating {
            self.inner.push(item);
            self.inner.len() - 1
        } else {
            match self.do_insert(item) {
                Ok(pos) | Err(pos) => pos,
            }
        };
        &self.inner[pos]
    }

    fn sorted_slice(&self) -> &[T] {
        if self.evaluating {
            &self.inner[..self.additions_index]
        } else {
            self.inner.as_slice()
        }
    }

    fn do_insert(&mut self, item: T) -> Result<usize, usize> {
        let pos = match self.sorted_slice().binary_search(&item) {
            Ok(pos) => {
                self.inner[pos] = item;
                return Ok(pos);
            }
            Err(pos) => pos,
        };
        self.inner.insert(pos, item);
        Err(pos)
    }

    pub fn replace(&mut self, i: usize, item: T) -> &T {
        let mut pos = match self.sorted_slice().binary_search(&item) {
            Ok(pos) | Err(pos) => pos,
        };
        if pos == i {
            let entry = &mut self.inner[i];
            *entry = item;
            return entry;
        }
        if self.evaluating {
            self.removals.push(i);
            self.inner.push(item);
            return &self.inner[self.inner.len() - 1];
        }
        self.inner[i] = item;
        if pos == self.inner.len() {
            pos -= 1;
        }
        match i.cmp(&pos) {
            Ordering::Less => self.inner[i..=pos].rotate_left(1),
            Ordering::Equal => (),
            Ordering::Greater => self.inner[pos..=i].rotate_right(1),
        }
        &self.inner[pos]
    }

    pub fn remove(&mut self, i: usize) {
        if self.evaluating && i <= self.cursor {
            self.removals.push(i);
        } else {
            self.inner.remove(i);
        }
    }

    pub fn remove_current(&mut self) {
        self.removals.push(self.cursor);
    }

    pub fn find<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<&T> {
        let (_, item) = self
            .inner
            .iter()
            .enumerate()
            .find(|(i, item)| pred(item) && !self.removals.contains(i))?;
        Some(item)
    }

    pub fn find_mut<P: FnMut(&T) -> bool>(&mut self, mut pred: P) -> Option<&mut T> {
        let (_, item) = self
            .inner
            .iter_mut()
            .enumerate()
            .find(|(i, item)| pred(item) && !self.removals.contains(i))?;
        Some(item)
    }

    pub fn position<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<usize> {
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
