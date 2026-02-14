use std::ops::Deref;
use std::pin::Pin;

use cxx_qt::casting::Upcast;

use crate::QWidget;

#[cxx_qt::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qsize.h");
        type QSize = cxx_qt_lib::QSize;

        include!("smushclient-qt-lib/qwidget.h");
        type QWidget = crate::QWidget;
    }

    unsafe extern "C++Qt" {
        include!("smushclient-qt-lib/qabstractscrollarea.h");

        #[qobject]
        #[base = QWidget]
        type QAbstractScrollArea;

        /// Returns the size of the viewport as if the scroll bars had no valid scrolling range.
        #[rust_name = "maximum_viewport_size"]
        fn maximumViewportSize(self: &QAbstractScrollArea) -> QSize;
    }
}

pub use ffi::QAbstractScrollArea;

impl QAbstractScrollArea {
    pub fn as_widget(&self) -> &QWidget {
        self.upcast()
    }

    pub fn as_widget_mut(self: Pin<&mut Self>) -> Pin<&mut QWidget> {
        self.upcast_pin()
    }
}

impl Deref for QAbstractScrollArea {
    type Target = QWidget;

    fn deref(&self) -> &Self::Target {
        self.upcast()
    }
}
