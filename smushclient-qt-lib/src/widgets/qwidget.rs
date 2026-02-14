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

        /// Returns the font metrics for the widget's current font. Equivalent to `QFontMetrics::new(font())`.
        #[rust_name = "font_metrics"]
        fn fontMetrics(self: &QWidget) -> QFontMetrics;
    }
}

pub use ffi::QWidget;
