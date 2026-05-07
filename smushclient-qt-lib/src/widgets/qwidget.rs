use crate::QFontMetrics;
use crate::util::new_in_place;

#[cxx_qt::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qfont.h");
        type QFont = cxx_qt_lib::QFont;
        include!("cxx-qt-lib/qmargins.h");
        type QMargins = cxx_qt_lib::QMargins;

        include!("smushclient-qt-lib/qfontmetrics.h");
        type QFontMetrics = crate::QFontMetrics;
    }

    unsafe extern "C++Qt" {
        include!("smushclient-qt-lib/qwidget.h");

        #[qobject]
        type QWidget;

        /// Returns the widget's contents margins.
        #[rust_name = "contents_margins"]
        fn contentsMargins(self: &QWidget) -> QMargins;

        /// This property describes the widget's requested font. The font is used by the widget's style when rendering standard components, and is available as a means to ensure that custom widgets can maintain consistency with the native platform's look and feel. It's common that different platforms, or different styles, define different fonts for an application.
        fn font(self: &QWidget) -> &QFont;
    }

    #[namespace = "rust::smushclientqtlib1"]
    unsafe extern "C++" {
        #[rust_name = "qwidget_font_metrics"]
        unsafe fn qwidgetFontMetrics(widget: &QWidget, uninit: *mut QFontMetrics);
    }
}

pub use ffi::QWidget;

impl QWidget {
    /// Returns the font metrics for the widget's current font. Equivalent to `QFontMetrics::new(font())`.
    pub fn font_metrics(self: &QWidget) -> QFontMetrics {
        // SAFETY: ffi:: initializes the passed pointer in-place.
        unsafe { new_in_place(|p| ffi::qwidget_font_metrics(self, p)) }
    }
}
