use smushclient::TimerStart;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qset.h");
        type QSet_u16 = cxx_qt_lib::QSet<u16>;
    }

    extern "C++" {
        include!("smushclient_qt/src/ffi/send_request.cxx.h");
        type SendRequest = super::super::send_request::ffi::SendRequest;
    }

    struct SendTimer {
        #[cxx_name = "activeClosed"]
        active_closed: bool,
        label: String,
        script: String,
        request: SendRequest,
    }

    unsafe extern "C++Qt" {
        include!("smushclient_qt/timekeeper.h");
        type SendTarget = crate::ffi::SendTarget;

        #[qobject]
        type Timekeeper;

        #[rust_name = "cancel_timers"]
        fn cancelTimers(self: Pin<&mut Timekeeper>, timer_ids: &QSet_u16);

        #[rust_name = "send_timer"]
        fn sendTimer(self: &Timekeeper, timer: &SendTimer);

        #[rust_name = "set_open"]
        fn setOpen(self: Pin<&mut Timekeeper>, open: bool);

        #[doc(hidden)]
        #[rust_name = "start_send_timer"]
        fn startSendTimer(self: &Timekeeper, index: usize, timer: u16, milliseconds: u32);
    }

    impl UniquePtr<Timekeeper> {}
}

impl ffi::Timekeeper {
    pub fn start(&self, start: &TimerStart) {
        self.start_send_timer(start.plugin, start.timer, start.milliseconds);
    }
}
