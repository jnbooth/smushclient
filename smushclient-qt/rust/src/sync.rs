use std::sync::atomic::{AtomicBool, Ordering};

#[derive(Default)]
#[repr(transparent)]
pub struct SimpleLock {
    locked: AtomicBool,
}

impl SimpleLock {
    pub fn lock(&self) -> SimpleLockGuard {
        if self.locked.swap(true, Ordering::Relaxed) {
            panic!("concurrent access");
        }
        SimpleLockGuard {
            locked: &self.locked,
        }
    }
}

pub struct SimpleLockGuard<'a> {
    locked: &'a AtomicBool,
}

impl<'a> Drop for SimpleLockGuard<'a> {
    fn drop(&mut self) {
        self.locked.store(false, Ordering::Relaxed);
    }
}
