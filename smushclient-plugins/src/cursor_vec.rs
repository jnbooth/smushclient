use std::cell::{Cell, Ref, RefCell, RefMut};
use std::cmp::Ordering;
use std::iter::FusedIterator;
use std::ops::Range;

use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct CursorVec<T> {
    inner: RefCell<Vec<T>>,
    cursor: Cell<usize>,
    removals: RefCell<Vec<usize>>,
    sorted_until: Cell<usize>,
    need_full_resort: Cell<bool>,
    evaluating: Cell<bool>,
}

impl<T> CursorVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: RefCell::new(Vec::new()),
            cursor: Cell::new(0),
            removals: RefCell::new(Vec::new()),
            sorted_until: Cell::new(0),
            need_full_resort: Cell::new(false),
            evaluating: Cell::new(false),
        }
    }
}

#[derive(Debug)]
pub struct CursorVecScan<'a, T: Ord> {
    vec: &'a CursorVec<T>,
}

impl<T: Ord> Drop for CursorVecScan<'_, T> {
    fn drop(&mut self) {
        self.vec.end();
    }
}

impl<'a, T: Ord> Iterator for CursorVecScan<'a, T> {
    type Item = CursorVecRef<'a, T>;

    fn next(&mut self) -> Option<Self::Item> {
        self.vec.next()
    }
}

impl<T: Ord> FusedIterator for CursorVecScan<'_, T> {}

impl<T: Ord> CursorVec<T> {
    pub fn is_empty(&self) -> bool {
        self.inner.borrow().is_empty()
    }

    pub fn borrow(&self) -> Ref<'_, Vec<T>> {
        self.inner.borrow()
    }

    pub fn len(&self) -> usize {
        self.inner.borrow().len()
    }

    pub fn borrow_mut(&self) -> RefMut<'_, Vec<T>> {
        self.inner.borrow_mut()
    }

    pub fn get(&self, i: usize) -> Option<Ref<'_, T>> {
        Ref::filter_map(self.inner.borrow(), |inner| inner.get(i)).ok()
    }

    pub fn iter(&self) -> Iter<'_, T> {
        self.into_iter()
    }

    pub fn scan(&self) -> CursorVecScan<'_, T> {
        #[track_caller]
        #[cold]
        fn panic_already_open() -> ! {
            panic!("CursorVec already scanning");
        }
        if self.evaluating.get() {
            panic_already_open();
        }
        self.evaluating.set(true);
        self.sorted_until.set(self.inner.borrow().len());
        self.cursor.set(0);
        self.need_full_resort.set(false);
        CursorVecScan { vec: self }
    }

    pub fn end(&self) {
        if !self.evaluating.replace(false) {
            return;
        }
        let mut inner = self.inner.borrow_mut();
        let mut removals = self.removals.borrow_mut();
        let mut sorted_until = self.sorted_until.get();
        for &i in removals.iter().rev() {
            inner.remove(i);
            sorted_until -= 1;
        }
        removals.clear();
        if self.need_full_resort.get() {
            inner.sort_unstable();
            return;
        }
        while sorted_until < inner.len() {
            let Some(item) = inner.pop() else {
                break;
            };
            if Self::do_insert_into(&mut inner, sorted_until, item).is_err() {
                sorted_until += 1;
            }
        }
    }

    fn next(&self) -> Option<CursorVecRef<'_, T>> {
        if !self.evaluating.get() {
            return None;
        }
        let mut inner = self.inner.borrow_mut();
        let mut cursor = self.cursor.get();
        if cursor >= inner.len() {
            return None;
        }
        let mut removals = self.removals.borrow_mut();
        let mut sorted_until = self.sorted_until.get();
        for &i in removals.iter().rev() {
            inner.remove(i);
            sorted_until -= 1;
            if i <= cursor {
                cursor -= 1;
            }
        }
        removals.clear();
        while sorted_until < inner.len() {
            let Some(item) = inner.pop() else {
                break;
            };
            let Err(pos) = Self::do_insert_into(&mut inner, sorted_until, item) else {
                continue;
            };
            sorted_until += 1;
            if pos < cursor {
                cursor += 1;
            }
        }
        cursor += 1;
        self.cursor.set(cursor);
        if cursor >= inner.len() {
            return None;
        }
        self.sorted_until.set(inner.len());
        Some(CursorVecRef {
            vec: self,
            index: cursor,
        })
    }

    pub fn append(&self, other: &mut Vec<T>) {
        self.inner.borrow_mut().append(other);
    }

    pub fn insert(&self, item: T) -> Ref<'_, T> {
        let mut inner = self.inner.borrow_mut();
        let pos = if self.evaluating.get() {
            inner.push(item);
            inner.len() - 1
        } else {
            let len = inner.len();
            match Self::do_insert_into(&mut inner, len, item) {
                Ok(pos) | Err(pos) => pos,
            }
        };
        let inner = self.inner.borrow();
        Ref::map(inner, |inner| &inner[pos])
    }

    fn do_insert_into(items: &mut Vec<T>, until: usize, item: T) -> Result<usize, usize> {
        let pos = match items[..until].binary_search(&item) {
            Ok(pos) => {
                items[pos] = item;
                return Ok(pos);
            }
            Err(pos) => pos,
        };
        items.insert(pos, item);
        Err(pos)
    }

    pub fn replace(&self, i: usize, item: T) -> (usize, Ref<'_, T>) {
        let pos = self.do_replace(i, item);
        let item = Ref::map(self.inner.borrow(), |inner| &inner[pos]);
        (pos, item)
    }

    fn do_replace(&self, i: usize, item: T) -> usize {
        let mut inner = self.inner.borrow_mut();
        let evaluating = self.evaluating.get();
        let sorted_slice = if evaluating {
            &inner[..self.sorted_until.get()]
        } else {
            &inner
        };
        if i >= sorted_slice.len() {
            inner[i] = item;
            return i;
        }
        let mut pos = match sorted_slice.binary_search(&item) {
            Ok(pos) | Err(pos) => pos,
        };
        inner[i] = item;
        if pos == i || pos == i + 1 {
            return i;
        }
        if evaluating && i <= self.cursor.get() {
            self.need_full_resort.set(true);
            return i;
        }
        if pos == inner.len() {
            pos -= 1;
        }
        match i.cmp(&pos) {
            Ordering::Less => inner[i..=pos].rotate_left(1),
            Ordering::Equal => (),
            Ordering::Greater => inner[pos..=i].rotate_right(1),
        }
        pos
    }

    pub fn remove(&self, i: usize) {
        if self.evaluating.get() && i <= self.cursor.get() {
            self.removals.borrow_mut().push(i);
        } else {
            self.inner.borrow_mut().remove(i);
        }
    }

    pub fn find<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<Ref<'_, T>> {
        let removals = self.removals.borrow();
        Ref::filter_map(self.inner.borrow(), |inner| {
            if removals.is_empty() {
                return inner.iter().find(|item| pred(item));
            }
            inner
                .iter()
                .enumerate()
                .find(|(i, item)| pred(item) && !removals.contains(i))
                .map(|(_, item)| item)
        })
        .ok()
    }

    pub fn find_mut<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<RefMut<'_, T>> {
        let removals = self.removals.borrow();
        RefMut::filter_map(self.inner.borrow_mut(), |inner| {
            if removals.is_empty() {
                return inner.iter_mut().find(|item| pred(item));
            }
            inner
                .iter_mut()
                .enumerate()
                .find(|(i, item)| pred(item) && !removals.contains(i))
                .map(|(_, item)| item)
        })
        .ok()
    }

    pub fn position<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<usize> {
        let removals = self.removals.borrow();
        if removals.is_empty() {
            return self.inner.borrow().iter().position(pred);
        }
        self.inner
            .borrow()
            .iter()
            .enumerate()
            .find(|(i, item)| pred(item) && !removals.contains(i))
            .map(|(i, _)| i)
    }

    pub fn retain<P: FnMut(&T) -> bool>(&self, mut pred: P) -> usize {
        let mut inner = self.inner.borrow_mut();
        if !self.evaluating.get() {
            let len = inner.len();
            inner.retain(pred);
            return len - inner.len();
        }
        let cursor = self.cursor.get();
        let mut removals = self.removals.borrow_mut();
        let mut removed = 0;
        let mut i = 0;
        inner.retain(|item| {
            i += 1;
            if pred(item) || removals.contains(&(i - 1)) {
                return true;
            }
            removed += 1;
            if i < cursor {
                removals.push(i - 1);
                return true;
            }
            false
        });
        removed
    }
}

impl<T> From<Vec<T>> for CursorVec<T> {
    fn from(value: Vec<T>) -> Self {
        Self {
            inner: RefCell::new(value),
            ..Self::new()
        }
    }
}

impl<T> From<CursorVec<T>> for Vec<T> {
    fn from(value: CursorVec<T>) -> Self {
        value.inner.into_inner()
    }
}

impl<T> Default for CursorVec<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> FromIterator<T> for CursorVec<T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> Self {
        Vec::from_iter(iter).into()
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

#[derive(Debug)]
pub struct CursorVecRef<'a, T> {
    vec: &'a CursorVec<T>,
    index: usize,
}

impl<T> CursorVecRef<'_, T> {
    pub fn borrow(&self) -> Ref<'_, T> {
        Ref::map(self.vec.inner.borrow(), |vec| &vec[self.index])
    }

    pub fn borrow_mut(&self) -> RefMut<'_, T> {
        RefMut::map(self.vec.inner.borrow_mut(), |vec| &mut vec[self.index])
    }

    pub fn remove(self) {
        self.vec.removals.borrow_mut().push(self.index);
    }
}

impl<'a, T> IntoIterator for &'a CursorVec<T> {
    type Item = CursorVecRef<'a, T>;

    type IntoIter = Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        Iter {
            vec: self,
            inner: 0..(self.inner.borrow().len()),
        }
    }
}

#[derive(Debug)]
pub struct Iter<'a, T> {
    vec: &'a CursorVec<T>,
    inner: Range<usize>,
}

impl<'a, T> Iterator for Iter<'a, T> {
    type Item = CursorVecRef<'a, T>;

    fn next(&mut self) -> Option<Self::Item> {
        let i = self.inner.next()?;
        Some(CursorVecRef {
            vec: self.vec,
            index: i,
        })
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        self.inner.size_hint()
    }

    fn count(self) -> usize {
        self.inner.count()
    }

    fn last(mut self) -> Option<Self::Item> {
        self.next_back()
    }

    fn nth(&mut self, n: usize) -> Option<Self::Item> {
        let i = self.inner.nth(n)?;
        Some(CursorVecRef {
            vec: self.vec,
            index: i,
        })
    }
}

impl<T> FusedIterator for Iter<'_, T> {}

impl<T> ExactSizeIterator for Iter<'_, T> {
    fn len(&self) -> usize {
        self.inner.len()
    }
}

impl<T> DoubleEndedIterator for Iter<'_, T> {
    fn next_back(&mut self) -> Option<Self::Item> {
        let i = self.inner.next_back()?;
        Some(CursorVecRef {
            vec: self.vec,
            index: i,
        })
    }

    fn nth_back(&mut self, n: usize) -> Option<Self::Item> {
        let i = self.inner.nth_back(n)?;
        Some(CursorVecRef {
            vec: self.vec,
            index: i,
        })
    }
}
