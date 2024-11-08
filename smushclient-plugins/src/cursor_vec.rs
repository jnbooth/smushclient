use std::ops::{Deref, DerefMut};
use std::{slice, vec};

use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CursorVec<T> {
    inner: Vec<T>,
    cursor: usize,
}

impl<T> CursorVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: Vec::new(),
            cursor: 0,
        }
    }

    pub fn restart(&mut self) {
        self.cursor = 0;
    }

    #[allow(clippy::should_implement_trait)]
    pub fn next(&mut self) -> Option<&mut T> {
        let item = self.inner.get_mut(self.cursor)?;
        self.cursor += 1;
        Some(item)
    }
}

impl<T: Ord> CursorVec<T> {
    pub fn iter(&self) -> slice::Iter<T> {
        self.inner.iter()
    }

    pub fn iter_mut(&mut self) -> slice::IterMut<T> {
        self.inner.iter_mut()
    }

    pub fn insert(&mut self, item: T) -> usize {
        let pos = match self.inner.binary_search(&item) {
            Ok(pos) => {
                self.inner[pos] = item;
                return pos;
            }
            Err(pos) => pos,
        };
        if pos == self.inner.len() {
            self.inner.push(item);
            return pos;
        }
        if pos < self.cursor {
            self.cursor += 1;
        }
        self.inner.insert(pos, item);
        pos
    }

    pub fn append(&mut self, other: &mut Vec<T>) {
        self.cursor = 0;
        self.inner.append(other);
        self.inner.sort_unstable();
    }

    pub fn remove(&mut self, i: usize) -> T {
        if i <= self.cursor {
            self.cursor -= 1;
        }
        self.inner.remove(i)
    }

    pub fn remove_current(&mut self) -> T {
        self.remove(self.cursor)
    }

    pub fn remove_if<P: FnMut(&T) -> bool>(&mut self, mut pred: P) -> usize {
        let len = self.inner.len();
        for i in (0..len).rev() {
            if pred(&self.inner[i]) {
                self.remove(i);
            }
        }
        len - self.inner.len()
    }
}

impl<T> From<Vec<T>> for CursorVec<T> {
    fn from(value: Vec<T>) -> Self {
        Self {
            inner: value,
            cursor: 0,
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
