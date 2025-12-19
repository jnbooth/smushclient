use std::pin::Pin;

use smushclient::TimerStart;

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

impl ffi::Timekeeper {
    pub fn start(self: Pin<&mut Self>, start: &TimerStart) {
        self.start_send_timer(start.index, start.timer, start.milliseconds);
    }
}
