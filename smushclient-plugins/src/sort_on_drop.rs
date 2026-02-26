use std::cell::RefMut;
use std::ops::Deref;
use std::slice;

#[derive(Debug)]
#[doc(hidden)]
pub struct SortOnDrop<'a, T: Ord> {
    senders: RefMut<'a, Vec<T>>,
    index: usize,
}

impl<'a, T: Ord> SortOnDrop<'a, T> {
    pub fn new(senders: RefMut<'a, Vec<T>>, index: usize) -> Self {
        Self { senders, index }
    }
}

impl<T: Ord> Drop for SortOnDrop<'_, T> {
    fn drop(&mut self) {
        self.senders.sort_unstable();
    }
}

impl<T: Ord> Deref for SortOnDrop<'_, T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        &self.senders[self.index..]
    }
}

impl<'a, T: Ord> IntoIterator for &'a SortOnDrop<'a, T> {
    type Item = &'a T;

    type IntoIter = slice::Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.senders[self.index..].iter()
    }
}
