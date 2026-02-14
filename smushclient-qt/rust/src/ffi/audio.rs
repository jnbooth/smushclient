use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient::AudioFilePlayback;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        type RustFilePlayback = super::RustFilePlaybackRust;

        pub fn play(self: &RustFilePlayback);
        pub fn set_file(self: Pin<&mut RustFilePlayback>, file: &QString) -> QString;
    }
}

impl ffi::RustFilePlayback {
    pub fn play(&self) {
        self.rust().play();
    }

    pub fn set_file(self: Pin<&mut Self>, path: &QString) -> QString {
        self.rust_mut().set_file(path)
    }
}

pub struct RustFilePlaybackRust {
    inner: AudioFilePlayback,
}

impl Default for RustFilePlaybackRust {
    /// # Panics
    ///
    /// Panics if audio initialization fails.
    fn default() -> Self {
        Self {
            inner: AudioFilePlayback::try_default().expect("audio initialization failed"),
        }
    }
}

impl RustFilePlaybackRust {
    pub fn play(&self) {
        self.inner.play();
    }

    pub fn set_file(&mut self, path: &QString) -> QString {
        let path = String::from(path);
        if path.is_empty() {
            self.inner.clear();
            return QString::default();
        }
        if let Err(e) = self.inner.set_file(path) {
            QString::from(&e.to_string())
        } else {
            QString::default()
        }
    }
}
