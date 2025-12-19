use std::ops::Deref;
use std::slice;

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(transparent)]
pub struct SortOnDrop<T: Ord> {
    inner: [T],
}

impl<T: Ord> SortOnDrop<T> {
    pub const fn borrow_mut(s: &mut [T]) -> &mut Self {
        // SAFETY: #[repr(transparent)]
        unsafe { &mut *(std::ptr::from_mut(s) as *mut Self) }
    }
}

impl<T: Ord> Deref for SortOnDrop<T> {
    type Target = [T];

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl<T: Ord> Drop for SortOnDrop<T> {
    fn drop(&mut self) {
        self.inner.sort_unstable();
    }
}

impl<'a, T: Ord> IntoIterator for &'a SortOnDrop<T> {
    type Item = &'a T;

    type IntoIter = slice::Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.inner.iter()
    }
}
