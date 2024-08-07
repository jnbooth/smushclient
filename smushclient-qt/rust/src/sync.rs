use std::sync::atomic::{AtomicBool, Ordering};

#[derive(Default)]
#[repr(transparent)]
pub struct NonBlockingMutex {
    locked: AtomicBool,
}

impl NonBlockingMutex {
    pub fn lock(&self) -> NonBlockingMutexGuard {
        if self.locked.swap(true, Ordering::Relaxed) {
            panic!("concurrent access");
        }
        NonBlockingMutexGuard {
            locked: &self.locked,
        }
    }
}

pub struct NonBlockingMutexGuard<'a> {
    locked: &'a AtomicBool,
}

impl<'a> Drop for NonBlockingMutexGuard<'a> {
    fn drop(&mut self) {
        self.locked.store(false, Ordering::Relaxed);
    }
}
