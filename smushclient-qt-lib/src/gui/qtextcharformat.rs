use std::fmt;
use std::mem::MaybeUninit;
use std::ops::{Deref, DerefMut};

use cxx::{ExternType, type_id};
use cxx_qt::casting::Upcast;
use cxx_qt_lib::{
    QColor, QFont, QFontCapitalization, QFontHintingPreference, QFontSpacingType, QFontStyleHint,
    QFontStyleStrategy, QPen, QString, QStringList,
};

use crate::{QTextFormat, QTextFormatProperty};

#[cxx::bridge]
mod ffi {
    #[doc(hidden)]
    #[repr(i32)]
    enum QFontWeight {
        Thin = 100,
        ExtraLight = 200,
        Light = 300,
        Normal = 400,
        Medium = 500,
        DemiBold = 600,
        Bold = 700,
        ExtraBold = 800,
        Black = 900,
    }

    /// This enum describes the different ways drawing underlined text.
    #[repr(i32)]
    #[derive(Debug)]
    enum QTextCharFormatUnderlineStyle {
        /// Text is draw without any underlining decoration.
        NoUnderline,
        /// A line is drawn using [`PenStyle::SolidLine`](cxx_qt_lib::PenStyle::SolidLine).
        SingleUnderline,
        /// A line is drawn using [`PenStyle::DashLine`](cxx_qt_lib::PenStyle::DashLine).
        DashUnderline,
        /// A line is drawn using [`PenStyle::DotLine`](cxx_qt_lib::PenStyle::DotLine).
        DotLine,
        /// A line is drawn using [`PenStyle::DashDotLine`](cxx_qt_lib::PenStyle::DashDotLine).
        DashDotLine,
        /// A line is drawn using [`PenStyle::DashDotDotLine`](cxx_qt_lib::PenStyle::DashDotDotLine).
        DashDotDotLine,
        /// The text is underlined using a wave shaped line.
        WaveUnderline,
        /// The underline is drawn depending on the SpellCheckUnderlineStyle theme hint of QPlatformTheme. By default this is mapped to [`QTextCharFormatUnderlineStyle::WaveUnderline`], on macOS it is mapped to [`QTextCharFormatUnderlineStyle::DotLine`].
        SpellCheckUnderline,
    }

    /// This enum describes the ways that adjacent characters can be vertically aligned.
    #[repr(i32)]
    #[derive(Debug)]
    enum QTextCharFormatVerticalAlignment {
        /// Adjacent characters are positioned in the standard way for text in the writing system in use.
        AlignNormal,
        /// Characters are placed above the base line for normal text.
        AlignSuperScript,
        /// Characters are placed below the base line for normal text.
        AlignSubScript,
        /// The center of the object is vertically aligned with the base line. Currently, this is only implemented for inline objects.
        AlignMiddle,
        //// The top edge of the object is vertically aligned with the base line.
        AlignTop,
        /// The bottom edge of the object is vertically aligned with the base line.
        AlignBottom,
        /// The base lines of the characters are aligned.
        AlignBaseline,
    }

    extern "C++" {
        include!("cxx-qt-lib/qfont.h");
        type QFont = cxx_qt_lib::QFont;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;
        include!("cxx-qt-lib/qtypes.h");
        type qreal = cxx_qt_lib::qreal;

        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qpen.h");
        type QPen = cxx_qt_lib::QPen;

        include!("smushclient-qt-lib/qtextformat.h");
        type QTextFormat = crate::QTextFormat;
    }

    #[namespace = "rust::cxxqtlib1"]
    extern "C++" {
        type QFontCapitalization = cxx_qt_lib::QFontCapitalization;
        type QFontHintingPreference = cxx_qt_lib::QFontHintingPreference;
        type QFontStyleHint = cxx_qt_lib::QFontStyleHint;
        type QFontStyleStrategy = cxx_qt_lib::QFontStyleStrategy;
        type QFontSpacingType = cxx_qt_lib::QFontSpacingType;
        type QFontWeight;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qtextcharformat.h");
        type QTextCharFormatUnderlineStyle;
        type QTextCharFormatVerticalAlignment;
    }

    unsafe extern "C++" {
        type QTextCharFormat = super::QTextCharFormat;

        /// Returns the anchor names associated with this text format, or an empty string list if none has been set. If the anchor names are set, text with this format can be the destination of a hypertext link.
        #[rust_name = "anchor_names"]
        fn anchorNames(&self) -> QStringList;

        /// Returns the font for this character format.
        fn font(&self) -> QFont;

        /// Sets the font style `hint` and `strategy`.
        #[rust_name = "set_font_style_hint"]
        fn setFontStyleHint(&mut self, hint: QFontStyleHint, strategy: QFontStyleStrategy);

        /// Sets the pen used to draw the outlines of characters to the given `pen`.
        #[rust_name = "set_text_outline"]
        fn setTextOutline(&mut self, pen: &QPen);
    }

    #[namespace = "rust::smushclientqtlib1"]
    unsafe extern "C++" {
        #[rust_name = "qtextcharformat_set_font"]
        fn qtextcharformatSetFont(format: &mut QTextCharFormat, font: &QFont);

        #[rust_name = "qtextcharformat_set_font_with_specified"]
        fn qtextcharformatSetFontWithSpecified(format: &mut QTextCharFormat, font: &QFont);

        #[rust_name = "qtextformat_to_char_format"]
        fn qtextformatToCharFormat(format: &QTextFormat) -> QTextCharFormat;
    }

    #[namespace = "rust::cxxqt1"]
    unsafe extern "C++" {
        include!("cxx-qt/casting.h");

        #[doc(hidden)]
        #[rust_name = "upcast_qtextcharformat"]
        unsafe fn upcastPtr(thiz: *const QTextCharFormat) -> *const QTextFormat;

        #[doc(hidden)]
        #[rust_name = "downcast_qtextformat"]
        unsafe fn downcastPtrStatic(base: *const QTextFormat) -> *const QTextCharFormat;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qtextcharformat_drop"]
        fn drop(config: &mut QTextCharFormat);

        #[rust_name = "qtextcharformat_init_default"]
        fn construct() -> QTextCharFormat;
        #[rust_name = "qtextcharformat_clone"]
        fn construct(other: &QTextCharFormat) -> QTextCharFormat;

        #[rust_name = "qtextcharformat_eq"]
        fn operatorEq(a: &QTextCharFormat, b: &QTextCharFormat) -> bool;

        #[rust_name = "qtextcharformat_to_debug_qstring"]
        fn toDebugQString(value: &QTextCharFormat) -> QString;
    }
}

pub use ffi::{QTextCharFormatUnderlineStyle, QTextCharFormatVerticalAlignment};

pub struct QFontWeight;

/// Qt uses a weighting scale from 1 to 1000 compatible with OpenType. A weight of 1 will be thin, whilst 1000 will be extremely black.
///
/// This enum contains predefined font weights.
#[allow(non_upper_case_globals)]
impl QFontWeight {
    pub const Thin: i32 = ffi::QFontWeight::Thin.repr;
    pub const ExtraLight: i32 = ffi::QFontWeight::ExtraLight.repr;
    pub const Light: i32 = ffi::QFontWeight::Light.repr;
    pub const Normal: i32 = ffi::QFontWeight::Normal.repr;
    pub const Medium: i32 = ffi::QFontWeight::Medium.repr;
    pub const DemiBold: i32 = ffi::QFontWeight::DemiBold.repr;
    pub const Bold: i32 = ffi::QFontWeight::Bold.repr;
    pub const ExtraBold: i32 = ffi::QFontWeight::ExtraBold.repr;
    pub const Black: i32 = ffi::QFontWeight::Black.repr;
}

/// The `QTextCharFormat` class provides formatting information for characters in a `QTextDocument`.
///
/// Qt Documentation: [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html#details)
#[repr(C)]
pub struct QTextCharFormat {
    _space: MaybeUninit<usize>,
    format_type: i32,
}

impl Clone for QTextCharFormat {
    fn clone(&self) -> Self {
        ffi::qtextcharformat_clone(self)
    }
}

impl Default for QTextCharFormat {
    /// Default constructs a `QTextCharFormat` object.
    fn default() -> Self {
        ffi::qtextcharformat_init_default()
    }
}

impl Drop for QTextCharFormat {
    fn drop(&mut self) {
        ffi::qtextcharformat_drop(self);
    }
}

impl PartialEq for QTextCharFormat {
    fn eq(&self, other: &Self) -> bool {
        ffi::qtextcharformat_eq(self, other)
    }
}

impl Eq for QTextCharFormat {}

impl fmt::Debug for QTextCharFormat {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        ffi::qtextcharformat_to_debug_qstring(self).fmt(f)
    }
}

impl From<&QTextFormat> for QTextCharFormat {
    fn from(value: &QTextFormat) -> Self {
        ffi::qtextformat_to_char_format(value)
    }
}

impl QTextCharFormat {
    /// Returns the text format's hypertext link, or an empty string if none has been set.
    pub fn anchor_href(&self) -> QString {
        self.string_property(QTextFormatProperty::AnchorHref)
    }

    /// Returns the the baseline offset in %.
    pub fn baseline_offset(&self) -> f64 {
        self.double_property(QTextFormatProperty::TextBaselineOffset)
    }

    /// Returns the current capitalization type of the font.
    pub fn font_capitalization(&self) -> QFontCapitalization {
        QFontCapitalization {
            repr: self.int_property(QTextFormatProperty::FontCapitalization),
        }
    }

    /// Returns the text format's font families.
    pub fn font_families(&self) -> QStringList {
        self.property(QTextFormatProperty::FontFamilies)
            .value_or_default()
    }

    /// Returns `true` if the text format's font is fixed pitch; otherwise returns `false`.
    pub fn font_fixed_pitch(&self) -> bool {
        self.bool_property(QTextFormatProperty::FontFixedPitch)
    }

    /// Returns the hinting preference set for this text format.
    pub fn font_hinting_preference(&self) -> QFontHintingPreference {
        QFontHintingPreference {
            repr: self.int_property(QTextFormatProperty::FontHintingPreference),
        }
    }

    /// Returns `true` if the text format's font is italic; otherwise returns `false`.
    pub fn font_italic(&self) -> bool {
        self.bool_property(QTextFormatProperty::FontItalic)
    }

    /// Returns `true` if the font kerning is enabled.
    pub fn font_kerning(&self) -> bool {
        self.bool_property(QTextFormatProperty::FontKerning)
    }

    /// Returns the current letter spacing.
    pub fn font_letter_spacing(&self) -> f64 {
        self.double_property(QTextFormatProperty::FontLetterSpacing)
    }

    /// Returns the letter spacing type of this format.
    pub fn font_letter_spacing_type(&self) -> QFontSpacingType {
        QFontSpacingType {
            repr: self.int_property(QTextFormatProperty::FontLetterSpacing),
        }
    }

    /// Returns `true` if the text format's font is overlined; otherwise returns `false`.
    pub fn font_overline(&self) -> bool {
        self.bool_property(QTextFormatProperty::FontOverline)
    }

    /// Returns the font size used to display text in this format.
    pub fn font_point_size(&self) -> f64 {
        self.double_property(QTextFormatProperty::FontPointSize)
    }

    /// Returns the current font stretching.
    pub fn font_stretch(&self) -> i32 {
        self.int_property(QTextFormatProperty::FontStretch)
    }

    /// Returns `true` if the text format's font is struck out (has a horizontal line drawn through it); otherwise returns `false`.
    pub fn font_strike_out(&self) -> bool {
        self.bool_property(QTextFormatProperty::FontStrikeOut)
    }

    /// Returns the font style hint.
    pub fn font_style_hint(&self) -> QFontStyleHint {
        QFontStyleHint {
            repr: self.int_property(QTextFormatProperty::FontStyleHint),
        }
    }

    /// Returns the text format's font style name.
    pub fn font_style_name(&self) -> QStringList {
        self.property(QTextFormatProperty::FontStyleName)
            .value_or_default()
    }

    /// Returns the current font style strategy.
    pub fn font_style_strategy(&self) -> QFontStyleStrategy {
        QFontStyleStrategy {
            repr: self.int_property(QTextFormatProperty::FontStyleStrategy),
        }
    }

    /// Returns `true` if the text format's font is underlined; otherwise returns `false`.
    pub fn font_underline(&self) -> bool {
        self.bool_property(QTextFormatProperty::TextUnderlineStyle)
    }

    /// Returns the text format's font weight.
    pub fn font_weight(&self) -> i32 {
        self.int_property(QTextFormatProperty::FontWeight)
    }

    /// Returns the current word spacing value.
    pub fn font_word_spacing(&self) -> f64 {
        self.double_property(QTextFormatProperty::FontWordSpacing)
    }

    /// Returns `true` if the text is formatted as an anchor; otherwise returns `false`.
    pub fn is_anchor(&self) -> bool {
        self.bool_property(QTextFormatProperty::IsAnchor)
    }

    /// If `anchor` is `true`, text with this format represents an anchor, and is formatted in the appropriate way; otherwise the text is formatted normally. (Anchors are hyperlinks which are often shown underlined and in a different color from plain text.)
    ///
    /// The way the text is rendered is independent of whether or not the format has a valid anchor defined. Use [`set_anchor_href`](Self::set_anchor_href), and optionally [`set_anchor_names`](Self::set_anchor_names) to create a hypertext link.
    pub fn set_anchor(&mut self, anchor: bool) {
        self.set_property(QTextFormatProperty::IsAnchor, &anchor);
    }

    /// Sets the hypertext link for the text format to the given `value`. This is typically a URL like `"http://example.com/index.html"`.
    ///
    /// The anchor will be displayed with the `value` as its display text; if you want to display different text call [`set_anchor_names`](Self::set_anchor_names).
    ///
    /// To format the text as a hypertext link use [`set_anchor`](Self::set_anchor).
    pub fn set_anchor_href(&mut self, value: &QString) {
        self.set_property(QTextFormatProperty::AnchorHref, value);
    }

    /// Sets the text format's anchor names. For the anchor to work as a hyperlink, the destination must be set with [`set_anchor_href`](Self::set_anchor_href) and the anchor must be enabled with [`set_anchor`](Self::set_anchor).
    pub fn set_anchor_names(&mut self, names: &QStringList) {
        self.set_property(QTextFormatProperty::AnchorName, names);
    }

    /// Sets the base line (in % of height) of text to `baseline`. A positive value moves the text up, by the corresponding %; a negative value moves it down. The default value is 0.
    pub fn set_baseline_offset(&mut self, baseline: f64) {
        self.set_property(QTextFormatProperty::TextBaselineOffset, &baseline);
    }

    /// Sets the text format's `font`. The font property that has not been explicitly set is treated like as it were set with default value.
    pub fn set_font(&mut self, font: &QFont) {
        ffi::qtextcharformat_set_font(self, font);
    }

    /// Sets the text format's `font`. The font property that has not been explicitly set is ignored and the respective property value remains unchanged.
    pub fn set_font_merged(&mut self, font: &QFont) {
        ffi::qtextcharformat_set_font_with_specified(self, font);
    }

    /// Sets the capitalization of the text that appears in this font to `capitalization`.
    ///
    /// A font's capitalization makes the text appear in the selected capitalization mode.
    pub fn set_font_capitalization(&mut self, capitalization: QFontCapitalization) {
        self.set_property(
            QTextFormatProperty::FontCapitalization,
            &capitalization.repr,
        );
    }

    /// Sets the text format's font `families`.
    pub fn set_font_families(&mut self, families: &QStringList) {
        self.set_property(QTextFormatProperty::FontFamilies, families);
    }

    /// If `fixed_pitch` is `true`, sets the text format's font to be fixed pitch; otherwise a non-fixed pitch font is used.
    pub fn set_font_fixed_pitch(&mut self, fixed_pitch: bool) {
        self.set_property(QTextFormatProperty::FontFixedPitch, &fixed_pitch);
    }

    /// Sets the hinting preference of the text format's font to be `hinting_preference`.
    pub fn set_font_hinting_preference(&mut self, hinting_preference: QFontHintingPreference) {
        self.set_property(
            QTextFormatProperty::FontHintingPreference,
            &hinting_preference.repr,
        );
    }

    /// If `italic` is `true`, sets the text format's font to be italic; otherwise the font will be non-italic.
    pub fn set_font_italic(&mut self, italic: bool) {
        self.set_property(QTextFormatProperty::FontItalic, &italic);
    }

    /// Enables kerning for this font if `enable` is `true`; otherwise disables it.
    ///
    /// When kerning is enabled, glyph metrics do not add up anymore, even for Latin text. In other words, the assumption that width('a') + width('b') is equal to width("ab") is not neccesairly true.
    pub fn set_font_kerning(&mut self, enable: bool) {
        self.set_property(QTextFormatProperty::FontKerning, &enable);
    }

    /// Sets the letter spacing of this format to the given `spacing`. The meaning of the value depends on the font letter spacing type.
    ///
    /// For percentage spacing a value of 100 indicates default spacing; a value of 200 doubles the amount of space a letter takes.
    pub fn set_font_letter_spacing(&mut self, spacing: f64) {
        self.set_property(QTextFormatProperty::FontLetterSpacing, &spacing);
    }

    /// Sets the letter spacing type of this format to `letter_spacing_type`.
    pub fn set_font_letter_spacing_type(&mut self, letter_spacing_type: QFontSpacingType) {
        self.set_property(
            QTextFormatProperty::FontLetterSpacingType,
            &letter_spacing_type.repr,
        );
    }

    /// If `overline` is `true`, sets the text format's font to be overlined; otherwise the font is displayed non-overlined.
    pub fn set_font_overline(&mut self, overline: bool) {
        self.set_property(QTextFormatProperty::FontOverline, &overline);
    }

    /// Sets the text format's font `size`.
    pub fn set_font_point_size(&mut self, size: f64) {
        self.set_property(QTextFormatProperty::FontPointSize, &size);
    }

    /// Sets the stretch factor for the font to `factor`.
    ///
    /// The stretch factor changes the width of all characters in the font by factor percent. For example, setting factor to 150 results in all characters in the font being 1.5 times (ie. 150%) wider. The default stretch factor is 100. The minimum stretch factor is 1, and the maximum stretch factor is 4000.
    ///
    /// The stretch factor is only applied to outline fonts. The stretch factor is ignored for bitmap fonts.
    pub fn set_font_stretch(&mut self, factor: i32) {
        self.set_property(QTextFormatProperty::FontStretch, &factor);
    }

    /// If `strike_out` is `true`, sets the text format's font with strike-out enabled (with a horizontal line through it); otherwise it is displayed without strikeout.
    pub fn set_font_strike_out(&mut self, strike_out: bool) {
        self.set_property(QTextFormatProperty::FontStrikeOut, &strike_out);
    }

    /// Sets the text format's font `style_name`.
    pub fn set_font_style_name(&mut self, style_name: &QString) {
        self.set_property(QTextFormatProperty::FontStyleName, style_name);
    }

    /// Sets the font style `strategy`.
    pub fn set_font_style_strategy(&mut self, strategy: QFontStyleStrategy) {
        self.set_property(QTextFormatProperty::FontStyleStrategy, &strategy.repr);
    }

    /// If `underline` is true, sets the text format's font to be underlined; otherwise it is displayed non-underlined.
    pub fn set_font_underline(&mut self, underline: bool) {
        let style = if underline {
            QTextCharFormatUnderlineStyle::SingleUnderline
        } else {
            QTextCharFormatUnderlineStyle::NoUnderline
        };
        self.set_underline_style(style);
    }

    /// Sets the text format's font weight to `weight`.
    pub fn set_font_weight(&mut self, weight: i32) {
        self.set_property(QTextFormatProperty::FontWeight, &weight);
    }

    /// Sets the word spacing of this format to the given `spacing`, in pixels.
    pub fn set_font_word_spacing(&mut self, spacing: f64) {
        self.set_property(QTextFormatProperty::FontWordSpacing, &spacing);
    }

    /// Sets the subscript's base line as a % of font height to `baseline`. The default value is 16.67% (1/6 of height).
    pub fn set_sub_script_baseline(&mut self, baseline: f64) {
        self.set_property(QTextFormatProperty::TextSubScriptBaseline, &baseline);
    }

    /// Sets the superscript's base line as a % of font height to `baseline`. The default value is 50% (1/2 of height).
    pub fn set_super_script_baseline(&mut self, baseline: f64) {
        self.set_property(QTextFormatProperty::TextSuperScriptBaseline, &baseline);
    }

    /// Sets the tool tip for a fragment of text to the given `text`.
    pub fn set_tool_tip(&mut self, text: &QString) {
        self.set_property(QTextFormatProperty::TextToolTip, text);
    }

    /// Sets the color used to draw underlines, overlines and strikeouts on the characters with this format to the `color` specified.
    pub fn set_underline_color(&mut self, color: &QColor) {
        self.set_property(QTextFormatProperty::TextUnderlineColor, color);
    }

    /// Sets the style of underlining the text to `style`.
    pub fn set_underline_style(&mut self, style: QTextCharFormatUnderlineStyle) {
        self.set_property(QTextFormatProperty::TextUnderlineStyle, &style.repr);
    }

    /// Sets the vertical alignment used for the characters with this format to the `alignment` specified.
    pub fn set_vertical_alignment(&mut self, alignment: QTextCharFormatVerticalAlignment) {
        self.set_property(QTextFormatProperty::TextVerticalAlignment, &alignment.repr);
    }

    /// Returns the subscript's base line as a % of font height.
    pub fn sub_script_baseline(&self) -> f64 {
        self.double_property(QTextFormatProperty::TextSubScriptBaseline)
    }

    /// Returns the superscript's base line as a % of font height.
    pub fn super_script_baseline(&self) -> f64 {
        self.double_property(QTextFormatProperty::TextSuperScriptBaseline)
    }

    /// Returns the pen used to draw the outlines of characters in this format.
    pub fn text_outline(&self) -> QPen {
        self.pen_property(QTextFormatProperty::TextOutline)
    }

    /// Returns the tool tip that is displayed for a fragment of text.
    pub fn tool_tip(&self) -> QString {
        self.string_property(QTextFormatProperty::TextToolTip)
    }

    /// Returns the color used to draw underlines, overlines and strikeouts on the characters with this format.
    pub fn underline_color(&self) -> QColor {
        self.color_property(QTextFormatProperty::TextUnderlineColor)
    }

    /// Returns the style of underlining the text.
    pub fn underline_style(&self) -> QTextCharFormatUnderlineStyle {
        QTextCharFormatUnderlineStyle {
            repr: self.int_property(QTextFormatProperty::TextUnderlineStyle),
        }
    }

    /// Returns the vertical alignment used for characters with this format.
    pub fn vertical_alignment(&self) -> QTextCharFormatVerticalAlignment {
        QTextCharFormatVerticalAlignment {
            repr: self.int_property(QTextFormatProperty::TextVerticalAlignment),
        }
    }
}

impl Deref for QTextCharFormat {
    type Target = QTextFormat;

    fn deref(&self) -> &Self::Target {
        self.upcast()
    }
}

impl DerefMut for QTextCharFormat {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.upcast_mut()
    }
}

unsafe impl Upcast<QTextFormat> for QTextCharFormat {
    unsafe fn upcast_ptr(this: *const Self) -> *const QTextFormat {
        unsafe { ffi::upcast_qtextcharformat(this) }
    }

    unsafe fn from_base_ptr(base: *const QTextFormat) -> *const Self {
        unsafe { ffi::downcast_qtextformat(base) }
    }
}

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QTextCharFormat {
    type Id = type_id!("QTextCharFormat");
    type Kind = cxx::kind::Trivial;
}
