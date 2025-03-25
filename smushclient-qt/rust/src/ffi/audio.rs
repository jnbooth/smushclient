use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient::{AudioError, AudioPlayback};

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        type RustPlayback = super::RustPlaybackRust;

        pub fn play(self: &RustPlayback);
        pub fn set_file(self: Pin<&mut RustPlayback>, file: &QString);
    }
}

impl ffi::RustPlayback {
    pub fn play(&self) {
        self.rust().play();
    }

    pub fn set_file(self: Pin<&mut Self>, path: &QString) {
        self.rust_mut().set_file(path);
    }
}

pub struct RustPlaybackRust {
    inner: AudioPlayback,
}

impl Default for RustPlaybackRust {
    fn default() -> Self {
        Self {
            inner: AudioPlayback::try_default().unwrap(),
        }
    }
}

impl RustPlaybackRust {
    fn handle(result: Result<(), AudioError>) {
        if let Err(e) = result {
            eprintln!("audio error: {e}");
        }
    }

    pub fn play(&self) {
        Self::handle(self.inner.play());
    }

    pub fn set_file(&mut self, path: &QString) {
        let path = String::from(path);
        if path.is_empty() {
            self.inner.clear();
            return;
        }
        Self::handle(self.inner.set_file(path));
    }
}
