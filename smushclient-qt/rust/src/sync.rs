use std::sync::atomic::{AtomicBool, Ordering};

#[derive(Default)]
#[repr(transparent)]
pub struct NonBlockingMutex {
    locked: AtomicBool,
}

impl NonBlockingMutex {
    /// # Panics
    ///
    /// Panics if the mutex is already locked.
    pub fn lock(&self) -> NonBlockingMutexGuard<'_> {
        assert!(
            !self.locked.swap(true, Ordering::Relaxed),
            "concurrent access"
        );
        NonBlockingMutexGuard {
            locked: &self.locked,
        }
    }
}

pub struct NonBlockingMutexGuard<'a> {
    locked: &'a AtomicBool,
}

impl Drop for NonBlockingMutexGuard<'_> {
    fn drop(&mut self) {
        self.locked.store(false, Ordering::Relaxed);
    }
}
