use std::cell::{Cell, Ref, RefCell, RefMut};
use std::cmp::Ordering;
use std::iter::FusedIterator;

use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct CursorVec<T> {
    inner: RefCell<Vec<T>>,
    cursor: Cell<usize>,
    unsorted: Cell<bool>,
    evaluating: Cell<bool>,
    current_deleted: Cell<bool>,
}

impl<T> CursorVec<T> {
    pub const fn new() -> Self {
        Self {
            inner: RefCell::new(Vec::new()),
            cursor: Cell::new(0),
            unsorted: Cell::new(false),
            evaluating: Cell::new(false),
            current_deleted: Cell::new(false),
        }
    }
}

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
        self.cursor.set(usize::MAX);
        CursorVecScan { vec: self }
    }

    pub fn end(&self) {
        if !self.evaluating.replace(false) {
            return;
        }
        if self.unsorted.replace(false) {
            self.inner.borrow_mut().sort_unstable();
        }
    }

    pub fn append(&self, other: &mut Vec<T>) {
        self.inner.borrow_mut().append(other);
    }

    pub fn insert(&self, item: T) -> Ref<'_, T> {
        let pos = {
            let mut inner = self.inner.borrow_mut();
            match inner.binary_search(&item) {
                Ok(pos) => {
                    inner[pos] = item;
                    pos
                }
                Err(pos) => {
                    inner.insert(pos, item);
                    let cursor = self.cursor.get();
                    if pos <= cursor {
                        self.cursor.set(cursor + 1);
                    }
                    pos
                }
            }
        };
        Ref::map(self.inner.borrow(), |inner| &inner[pos])
    }

    pub fn replace(&self, i: usize, item: T) -> (usize, Ref<'_, T>) {
        let pos = self.do_replace(i, item);
        let item = Ref::map(self.inner.borrow(), |inner| &inner[pos]);
        (pos, item)
    }

    fn do_replace(&self, i: usize, item: T) -> usize {
        let mut inner = self.inner.borrow_mut();
        let mut pos = match inner.binary_search(&item) {
            Ok(pos) | Err(pos) => pos,
        };
        inner[i] = item;
        if pos == i || pos == i + 1 {
            return i;
        }
        if self.evaluating.get() {
            self.unsorted.set(true);
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
        self.inner.borrow_mut().remove(i);
        if !self.evaluating.get() {
            return;
        }
        let cursor = self.cursor.get();
        match i.cmp(&cursor) {
            Ordering::Less => self.cursor.set(cursor - 1),
            Ordering::Equal => self.current_deleted.set(true),
            Ordering::Greater => (),
        }
    }

    pub fn find<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<Ref<'_, T>> {
        Ref::filter_map(self.inner.borrow(), |inner| {
            inner.iter().find(|item| pred(item))
        })
        .ok()
    }

    pub fn find_mut<P: FnMut(&T) -> bool>(&self, mut pred: P) -> Option<RefMut<'_, T>> {
        RefMut::filter_map(self.inner.borrow_mut(), |inner| {
            inner.iter_mut().find(|item| pred(item))
        })
        .ok()
    }

    pub fn position<P: FnMut(&T) -> bool>(&self, pred: P) -> Option<usize> {
        self.inner.borrow().iter().position(pred)
    }

    pub fn retain<P: FnMut(&T) -> bool>(&self, mut pred: P) -> usize {
        let mut inner = self.inner.borrow_mut();
        if !self.evaluating.get() {
            let len = inner.len();
            inner.retain(pred);
            return len - inner.len();
        }
        let mut cursor = self.cursor.get();
        let len = inner.len();
        let mut i = 0;
        inner.retain(|item| {
            i += 1;
            if pred(item) {
                return true;
            }
            if i < cursor {
                cursor -= 1;
            }
            false
        });
        len - inner.len()
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
        if !self.vec.evaluating.get() {
            return None;
        }
        let cursor = if self.vec.current_deleted.get() {
            self.vec.cursor.get()
        } else {
            self.vec.cursor.get().wrapping_add(1)
        };
        if cursor >= self.vec.inner.borrow().len() {
            return None;
        }
        self.vec.cursor.set(cursor);
        Some(CursorVecRef { vec: self.vec })
    }
}

impl<T: Ord> FusedIterator for CursorVecScan<'_, T> {}

#[derive(Debug)]
pub struct CursorVecRef<'a, T> {
    vec: &'a CursorVec<T>,
}

impl<T: Ord> CursorVecRef<'_, T> {
    pub fn borrow(&self) -> Option<Ref<'_, T>> {
        if self.vec.current_deleted.get() {
            return None;
        }
        let cursor = self.vec.cursor.get();
        Some(Ref::map(self.vec.inner.borrow(), |vec| &vec[cursor]))
    }

    pub fn borrow_mut(&self) -> Option<RefMut<'_, T>> {
        if self.vec.current_deleted.get() {
            return None;
        }
        let cursor = self.vec.cursor.get();
        Some(RefMut::map(self.vec.inner.borrow_mut(), |vec| {
            &mut vec[cursor]
        }))
    }

    pub fn remove(self) {
        if self.vec.current_deleted.get() {
            return;
        }
        self.vec.remove(self.vec.cursor.get());
    }
}
