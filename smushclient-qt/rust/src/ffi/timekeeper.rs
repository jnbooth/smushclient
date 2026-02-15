use smushclient::TimerStart;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "C++Qt" {
        include!("smushclient_qt/timekeeper.h");
        type SendTarget = crate::ffi::SendTarget;

        #[qobject]
        type Timekeeper;

        #[doc(hidden)]
        #[rust_name = "start_send_timer"]
        fn startSendTimer(self: &Timekeeper, index: usize, timer: u16, milliseconds: u32);
    }
}

impl ffi::Timekeeper {
    pub fn start(&self, start: &TimerStart) {
        self.start_send_timer(start.plugin, start.timer, start.milliseconds);
    }
}
