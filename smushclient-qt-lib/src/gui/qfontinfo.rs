use std::mem::MaybeUninit;

use cxx::{ExternType, type_id};
use cxx_qt_lib::QFont;

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;

        include!("cxx-qt-lib/qfont.h");
        type QFont = cxx_qt_lib::QFont;

        include!("smushclient-qt-lib/qchar.h");
        type QChar = crate::QChar;
    }

    #[namespace = "rust::cxxqtlib1"]
    extern "C++" {
        type QFontStyle = cxx_qt_lib::QFontStyle;
        type QFontStyleHint = cxx_qt_lib::QFontStyleHint;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qfontinfo.h");
    }

    unsafe extern "C++" {
        type QFontInfo = super::QFontInfo;

        /// Returns `true` if [weight](https://doc.qt.io/qt/qfont.html#weight)() is a value greater than 400; otherwise returns `false`.
        fn bold(&self) -> bool;

        /// Returns `true` if the matched window system font is exactly the same as the one specified by the font; otherwise returns `false`.
        #[rust_name = "exact_match"]
        fn exactMatch(&self) -> bool;

        /// Returns the requested font family name.
        fn family(&self) -> QString;

        /// Returns `true` if fixed pitch has been set; otherwise returns `false`.
        #[rust_name = "fixed_pitch"]
        fn fixedPitch(&self) -> bool;

        /// Returns `true` if the [style](https://doc.qt.io/qt/qfont.html#style)() of the font is not [`QFontStyle::StyleNormal`].
        fn italic(&self) -> bool;

        /// Returns `true` if the font is overlined, otherwise `false`.
        fn overline(&self) -> bool;

        /// Returns the pixel size of the font if it was set with [`set_pixel_size`](Self::set_pixel_size). Returns -1 if the size was not specified in pixels.
        #[rust_name = "pixel_size"]
        fn pixelSize(&self) -> i32;

        /// Returns the point size of the font. Returns -1 if the font size was specified in pixels.
        #[rust_name = "point_size"]
        fn pointSize(&self) -> i32;

        /// Returns the point size of the font. Returns -1 if the font size was specified in pixels.
        #[rust_name = "point_size_f"]
        fn pointSizeF(&self) -> f64;

        /// Returns `true` if the font is struck out, otherwise `false`.
        #[rust_name = "strike_out"]
        fn strikeOut(&self) -> bool;

        /// Returns the style value of the matched window system font.
        fn style(&self) -> QFontStyle;

        /// Returns the style hint.
        #[rust_name = "style_hint"]
        fn styleHint(&self) -> QFontStyleHint;

        /// Returns the requested font style name. This can be used to match the font
        /// with irregular styles (that can't be normalized in other style properties).
        #[rust_name = "style_name"]
        fn styleName(&self) -> QString;

        /// Returns `true` if the font is underlined, otherwise `false`.
        fn underline(&self) -> bool;

        /// Returns the weight of the matched window system font.
        fn weight(&self) -> i32;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qfontinfo_drop"]
        fn drop(config: &mut QFontInfo);

        #[rust_name = "qfontinfo_clone"]
        fn construct(other: &QFontInfo) -> QFontInfo;
        #[rust_name = "qfontinfo_init_qfont"]
        fn construct(font: &QFont) -> QFontInfo;
    }
}

/// The `QFontInfo` class provides general information about fonts.
///
/// Qt Documentation: [QFontInfo](https://doc.qt.io/qt-6/qfontinfo.html#details)
#[repr(C)]
pub struct QFontInfo {
    _space: MaybeUninit<usize>,
}

impl Clone for QFontInfo {
    fn clone(&self) -> Self {
        ffi::qfontinfo_clone(self)
    }
}

impl Drop for QFontInfo {
    fn drop(&mut self) {
        ffi::qfontinfo_drop(self);
    }
}

impl From<&QFont> for QFontInfo {
    /// Constructs a font metrics object for `font`.
    ///
    /// The font metrics object holds the information for the font that is passed in the constructor at the time it is created, and is not updated if the font's attributes are changed later.
    fn from(font: &QFont) -> Self {
        ffi::qfontinfo_init_qfont(font)
    }
}

impl QFontInfo {
    /// Constructs a font info object for `font`.
    ///
    /// The font must be screen-compatible, i.e. a font you use when drawing text in widgets or pixmaps, not `QPicture` or `QPrinter`.
    ///
    /// The font info object holds the information for the font that is passed in the constructor at the time it is created, and is not updated if the font's attributes are changed later.
    pub fn new(font: &QFont) -> Self {
        ffi::qfontinfo_init_qfont(font)
    }
}

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QFontInfo {
    type Id = type_id!("QFontInfo");
    type Kind = cxx::kind::Trivial;
}
