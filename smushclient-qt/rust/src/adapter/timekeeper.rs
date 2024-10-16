use crate::ffi;

// SAFETY: Document instances are owned by their parent WorldTab, ensuring all pointers are valid.
adapter!(TimekeeperAdapter, ffi::Timekeeper);

impl<'a> TimekeeperAdapter<'a> {
    pub fn start_send_timer(&mut self, id: usize, milliseconds: u32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_send_timer(id, milliseconds) };
    }

    pub fn send_timer(&self, timer: &ffi::SendTimer) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.send_timer(timer) };
    }
}
