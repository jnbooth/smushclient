use smushclient::TimerStart;

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    extern "C++" {
        include!("smushclient_qt/src/ffi/send_request.cxx.h");
        type SendTarget = crate::ffi::SendTarget;
    }

    unsafe extern "C++" {
        include!("smushclient_qt/abstracttimekeeper.h");

        #[cxx_name = "AbstractTimekeeper"]
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
