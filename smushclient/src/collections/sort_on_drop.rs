use std::ops::Deref;
use std::slice;

#[derive(Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct SortOnDrop<'a, T: Ord> {
    inner: &'a mut [T],
}

impl<'a, T: Ord> Deref for SortOnDrop<'a, T> {
    type Target = &'a mut [T];

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<T: Ord> Drop for SortOnDrop<'_, T> {
    fn drop(&mut self) {
        self.inner.sort_unstable();
    }
}

impl<'a, T: Ord> From<&'a mut [T]> for SortOnDrop<'a, T> {
    fn from(value: &'a mut [T]) -> Self {
        Self { inner: value }
    }
}

impl<'a, T: Ord> IntoIterator for &'a SortOnDrop<'a, T> {
    type Item = &'a T;

    type IntoIter = slice::Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter()
    }
}
