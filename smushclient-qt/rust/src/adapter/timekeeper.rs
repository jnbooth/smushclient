use smushclient::TimerHandler;

use crate::ffi;

// SAFETY: Document instances are owned by their parent WorldTab, ensuring all pointers are valid.
adapter!(TimekeeperAdapter, ffi::Timekeeper);

impl<'a> TimerHandler<ffi::SendTimer> for TimekeeperAdapter<'a> {
    fn send_timer(&self, timer: &ffi::SendTimer) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.send_timer(timer) };
    }

    fn start_timer(&mut self, index: usize, timer: u16, milliseconds: u32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_send_timer(index, timer, milliseconds) };
    }
}
