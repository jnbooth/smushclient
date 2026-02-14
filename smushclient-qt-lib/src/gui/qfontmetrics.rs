use std::mem::MaybeUninit;

use cxx::{ExternType, type_id};
use cxx_qt_lib::{QFont, QString};

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qrect.h");
        type QRect = cxx_qt_lib::QRect;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;

        include!("cxx-qt-lib/qfont.h");
        type QFont = cxx_qt_lib::QFont;

        include!("smushclient-qt-lib/qchar.h");
        type QChar = crate::QChar;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qfontmetrics.h");
    }

    unsafe extern "C++" {
        type QFontMetrics = super::QFontMetrics;

        /// Returns the ascent of the font.
        ///
        /// The ascent of a font is the distance from the baseline to the highest position characters extend to. In practice, some font designers break this rule, e.g. when they put more than one accent on top of a character, or to accommodate a certain character, so it is possible (though rare) that this value will be too small.
        fn ascent(&self) -> i32;

        /// Returns the average width of glyphs in the font.
        #[rust_name = "average_char_width"]
        fn averageCharWidth(&self) -> i32;

        /// Returns the rectangle that is covered by ink if character `ch` were to be drawn at the origin of the coordinate system.
        ///
        /// Note that the bounding rectangle may extend to the left of (0, 0) (e.g., for italicized fonts), and that the text output may cover *all* pixels in the bounding rectangle. For a space character the rectangle will usually be empty.
        ///
        /// Note that the rectangle usually extends both above and below the base line.
        ///
        /// **Warning:** The width of the returned rectangle is not the advance width of the character. Use [`bounding_rect`](Self::bounding_rect) or [`horizontal_advance`](Self::horizontal_advance) instead.
        #[rust_name = "bounding_rect_char"]
        fn boundingRect(&self, ch: QChar) -> QRect;

        /// Returns the bounding rectangle of the characters in the string specified by `text`. The bounding rectangle always covers at least the set of pixels the text would cover if drawn at (0, 0).
        ///
        /// Note that the bounding rectangle may extend to the left of (0, 0), e.g. for italicized fonts, and that the width of the returned rectangle might be different than what the [`horizontal_advance`](Self::horizontal_advance) method returns.
        ///
        /// If you want to know the advance width of the string (to lay out a set of strings next to each other), use [`horizontal_advance`](Self::horizontal_advance) instead.
        ///
        /// Newline characters are processed as normal characters, not as linebreaks.
        ///
        /// The height of the bounding rectangle is at least as large as the value returned by [`height`](Self::height).
        #[rust_name = "bounding_rect"]
        fn boundingRect(&self, text: &QString) -> QRect;

        /// Returns the cap height of the font.
        ///
        /// The cap height of a font is the height of a capital letter above the baseline. It specifically is the height of capital letters that are flat - such as H or I - as opposed to round letters such as O, or pointed letters like A, both of which may display overshoot.
        #[rust_name = "cap_height"]
        fn capHeight(&self) -> i32;

        /// Returns the descent of the font.
        ///
        /// The descent is the distance from the base line to the lowest point characters extend to. In practice, some font designers break this rule, e.g. to accommodate a certain character, so it is possible (though rare) that this value will be too small.
        fn descent(&self) -> i32;

        /// Returns the font DPI.
        #[rust_name = "font_dpi"]
        fn fontDpi(&self) -> f64;

        /// Returns the height of the font.
        ///
        /// This is always equal to [`ascent`](Self::ascent) + [`descent`](Self::descent).
        fn height(&self) -> i32;

        /// Returns the horizontal advance in pixels of the first `len` characters of `text`. If `len` is negative (the default), the entire string is used. The entire length of `text` is analysed even if `len` is substantially shorter.
        #[rust_name = "horizontal_advance_first"]
        fn horizontalAdvance(&self, text: &QString, len: i32) -> i32;

        /// Returns the horizontal advance of character `ch` in pixels. This is a distance appropriate for drawing a subsequent character after `ch`.
        #[rust_name = "horizontal_advance_char"]
        fn horizontalAdvance(&self, ch: QChar) -> i32;

        /// Returns `true` if character `ch` is a valid character in the font; otherwise returns `false`.
        #[rust_name = "in_font"]
        fn inFont(&self, ch: QChar) -> bool;

        /// Returns `true` if the character `ucs4` encoded in UCS-4/UTF-32 is a valid character in the `font`; otherwise returns `false`.
        #[rust_name = "in_font_ucs4"]
        fn inFontUcs4(&self, ucs4: u32) -> bool;

        /// Returns the leading of the font.
        ///
        /// This is the natural inter-line spacing.
        fn leading(&self) -> i32;

        /// Returns the left bearing of character `ch` in the font.
        ///
        /// The left bearing is the right-ward distance of the left-most pixel of the character from the logical origin of the character. This value is negative if the pixels of the character extend to the left of the logical origin.
        #[rust_name = "left_bearing"]
        fn leftBearing(&self, ch: QChar) -> i32;

        /// Returns the distance from one base line to the next.
        ///
        /// This value is always equal to [`leading`](Self::leading) + [`height`](Self::height).
        #[rust_name = "line_spacing"]
        fn lineSpacing(&self) -> i32;

        /// Returns the width of the underline and strikeout lines, adjusted for the point size of the font.
        #[rust_name = "line_width"]
        fn lineWidth(&self) -> i32;

        /// Returns the width of the widest character in the font.
        #[rust_name = "max_width"]
        fn maxWidth(&self) -> i32;

        /// Returns the minimum left bearing of the font.
        ///
        /// This is the smallest [`left_bearing`](Self::left_bearing) of all characters in the font.
        ///
        /// Note that this function can be very slow if the font is large.
        #[rust_name = "min_left_bearing"]
        fn minLeftBearing(&self) -> i32;

        /// Returns the minimum right bearing of the font.
        ///
        /// This is the smallest [`right_bearing`](Self::right_bearing) of all characters in the font.
        ///
        /// Note that this function can be very slow if the font is large.
        #[rust_name = "min_right_bearing"]
        fn minRightBearing(&self) -> i32;

        /// Returns the distance from the base line to where an overline should be drawn.
        #[rust_name = "overline_pos"]
        fn overlinePos(&self) -> i32;

        /// Returns the right bearing of character `ch` in the font.
        ///
        /// The right bearing is the left-ward distance of the right-most pixel of the character from the logical origin of a subsequent character. This value is negative if the pixels of the character extend to the right of the [`horizontal_advance`](Self::horizontal_advance) of the character.
        #[rust_name = "right_bearing"]
        fn rightBearing(&self, ch: QChar) -> i32;

        /// Returns the distance from the base line to where the strikeout line should be drawn.
        #[rust_name = "strike_out_pos"]
        fn strikeOutPos(&self) -> i32;

        /// Returns a tight bounding rectangle around the characters in the string specified by `text`. The bounding rectangle always covers at least the set of pixels the text would cover if drawn at (0, 0).
        ///
        /// Note that the bounding rectangle may extend to the left of (0, 0), e.g. for italicized fonts, and that the width of the returned rectangle might be different than what the [`horizontal_advance`](Self::horizontal_advance) method returns.
        ///
        /// If you want to know the advance width of the string (to lay out a set of strings next to each other), use [`horizontal_advance`](Self::horizontal_advance) instead.
        ///
        /// Newline characters are processed as normal characters, not as linebreaks.
        #[rust_name = "tight_bounding_rect"]
        fn tightBoundingRect(&self, text: &QString) -> QRect;

        /// Returns the distance from the base line to where an underscore should be drawn.
        #[rust_name = "underline_pos"]
        fn underlinePos(&self) -> i32;

        /// Returns the 'x' height of the font. This is often but not always the same as the height of the character 'x'.
        #[rust_name = "x_height"]
        fn xHeight(&self) -> i32;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qfontmetrics_drop"]
        fn drop(config: &mut QFontMetrics);

        #[rust_name = "qfontmetrics_clone"]
        fn construct(other: &QFontMetrics) -> QFontMetrics;
        #[rust_name = "qfontmetrics_init_qfont"]
        fn construct(font: &QFont) -> QFontMetrics;

        #[rust_name = "qfontmetrics_eq"]
        fn operatorEq(a: &QFontMetrics, b: &QFontMetrics) -> bool;
    }
}

/// The `QFontMetrics` class provides font metrics information.
///
/// Qt Documentation: [QFontMetrics](https://doc.qt.io/qt-6/qfontmetrics.html#details)
#[repr(C)]
pub struct QFontMetrics {
    _space: MaybeUninit<usize>,
}

impl Clone for QFontMetrics {
    fn clone(&self) -> Self {
        ffi::qfontmetrics_clone(self)
    }
}

impl Drop for QFontMetrics {
    fn drop(&mut self) {
        ffi::qfontmetrics_drop(self);
    }
}

impl PartialEq for QFontMetrics {
    fn eq(&self, other: &Self) -> bool {
        ffi::qfontmetrics_eq(self, other)
    }
}

impl Eq for QFontMetrics {}

impl From<&QFont> for QFontMetrics {
    /// Constructs a font metrics object for `font`.
    ///
    /// The font metrics object holds the information for the font that is passed in the constructor at the time it is created, and is not updated if the font's attributes are changed later.
    fn from(font: &QFont) -> Self {
        ffi::qfontmetrics_init_qfont(font)
    }
}

impl QFontMetrics {
    /// Constructs a font metrics object for `font`.
    ///
    /// The font metrics object holds the information for the font that is passed in the constructor at the time it is created, and is not updated if the font's attributes are changed later.
    pub fn new(font: &QFont) -> Self {
        ffi::qfontmetrics_init_qfont(font)
    }

    /// Returns the horizontal advance in pixels of `text` laid out.
    ///
    /// The advance is the distance appropriate for drawing a subsequent character after `text`.
    pub fn horizontal_advance(&self, text: &QString) -> i32 {
        self.horizontal_advance_first(text, -1)
    }
}

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QFontMetrics {
    type Id = type_id!("QFontMetrics");
    type Kind = cxx::kind::Trivial;
}
