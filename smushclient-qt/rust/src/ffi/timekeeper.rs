use std::pin::Pin;

use smushclient::TimerHandler;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    struct SendTimer {
        #[rust_name = "active_closed"]
        activeClosed: bool,
        label: String,
        plugin: usize,
        script: String,
        target: SendTarget,
        text: QString,
        destination: QString,
    }

    unsafe extern "C++Qt" {
        include!("timekeeper.h");
        type SendTarget = crate::ffi::SendTarget;

        #[qobject]
        type Timekeeper;

        #[rust_name = "send_timer"]
        fn sendTimer(self: &Timekeeper, timer: &SendTimer);

        #[rust_name = "start_send_timer"]
        fn startSendTimer(self: Pin<&mut Timekeeper>, index: usize, timer: u16, milliseconds: u32);
    }
}

impl TimerHandler<ffi::SendTimer> for Pin<&mut ffi::Timekeeper> {
    fn send_timer(&self, timer: &ffi::SendTimer) {
        (**self).send_timer(timer);
    }

    fn start_timer(&mut self, index: usize, timer: u16, milliseconds: u32) {
        self.as_mut().start_send_timer(index, timer, milliseconds);
    }
}
