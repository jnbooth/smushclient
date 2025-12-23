use std::fmt;
use std::mem::MaybeUninit;

use cxx::{ExternType, type_id};
use cxx_qt_lib::{LayoutDirection, QColor, QFlags, QList, QPen, QString, QVariant};

#[cxx::bridge]
mod ffi {
    /// This enum describes the text item a [`QTextFormat`] object is formatting.
    #[repr(i32)]
    #[derive(Debug)]
    enum QTextFormatFormatType {
        /// An invalid format as created by the default constructor
        InvalidFormat = -1,
        /// The object formats a text block
        BlockFormat = 1,
        /// The object formats a single character
        CharFormat = 2,
        /// The object formats a list
        ListFormat = 3,
        /// The object formats a frame
        FrameFormat = 5,
        UserFormat = 100,
    }

    /// This enum describes the different properties a format can have.
    #[repr(i32)]
    enum QTextFormatProperty {
        ObjectIndex = 0x0,

        // paragraph and char
        CssFloat = 0x0800,
        LayoutDirection = 0x0801,

        OutlinePen = 0x810,
        BackgroundBrush = 0x820,
        ForegroundBrush = 0x821,
        BackgroundImageUrl = 0x823,

        // paragraph
        BlockAlignment = 0x1010,
        BlockTopMargin = 0x1030,
        BlockBottomMargin = 0x1031,
        BlockLeftMargin = 0x1032,
        BlockRightMargin = 0x1033,
        TextIndent = 0x1034,
        TabPositions = 0x1035,
        BlockIndent = 0x1040,
        LineHeight = 0x1048,
        LineHeightType = 0x1049,
        BlockNonBreakableLines = 0x1050,
        BlockTrailingHorizontalRulerWidth = 0x1060,
        HeadingLevel = 0x1070,
        BlockQuoteLevel = 0x1080,
        BlockCodeLanguage = 0x1090,
        BlockCodeFence = 0x1091,
        BlockMarker = 0x10A0,

        // character properties
        FontCapitalization = 0x1FE0,
        FontLetterSpacing = 0x1FE1,
        FontWordSpacing = 0x1FE2,
        FontStyleHint = 0x1FE3,
        FontStyleStrategy = 0x1FE4,
        FontKerning = 0x1FE5,
        FontHintingPreference = 0x1FE6,
        FontFamilies = 0x1FE7,
        FontStyleName = 0x1FE8,
        FontLetterSpacingType = 0x1FE9,
        FontStretch = 0x1FEA,
        FontFamily = 0x2000,
        FontPointSize = 0x2001,
        FontSizeAdjustment = 0x2002,
        FontWeight = 0x2003,
        FontItalic = 0x2004,
        #[deprecated = "use `QTextFormatProperty::TextUnderlineStyle` instead"]
        FontUnderline = 0x2005,
        FontOverline = 0x2006,
        FontStrikeOut = 0x2007,
        FontFixedPitch = 0x2008,
        FontPixelSize = 0x2009,

        TextUnderlineColor = 0x2020,
        TextVerticalAlignment = 0x2021,
        TextOutline = 0x2022,
        TextUnderlineStyle = 0x2023,
        TextToolTip = 0x2024,
        TextSuperScriptBaseline = 0x2025,
        TextSubScriptBaseline = 0x2026,
        TextBaselineOffset = 0x2027,

        IsAnchor = 0x2030,
        AnchorHref = 0x2031,
        AnchorName = 0x2032,

        // Included for backwards compatibility with old QDataStreams.
        // Should not be referenced in user code.
        #[doc(hidden)]
        OldFontLetterSpacingType = 0x2033,
        #[doc(hidden)]
        OldFontStretch = 0x2034,
        #[doc(hidden)]
        OldTextUnderlineColor = 0x2010,

        ObjectType = 0x2f00,

        // list properties
        ListStyle = 0x3000,
        ListIndent = 0x3001,
        ListNumberPrefix = 0x3002,
        ListNumberSuffix = 0x3003,

        // table and frame properties
        FrameBorder = 0x4000,
        FrameMargin = 0x4001,
        FramePadding = 0x4002,
        FrameWidth = 0x4003,
        FrameHeight = 0x4004,
        FrameTopMargin = 0x4005,
        FrameBottomMargin = 0x4006,
        FrameLeftMargin = 0x4007,
        FrameRightMargin = 0x4008,
        FrameBorderBrush = 0x4009,
        FrameBorderStyle = 0x4010,

        TableColumns = 0x4100,
        TableColumnWidthConstraints = 0x4101,
        TableCellSpacing = 0x4102,
        TableCellPadding = 0x4103,
        TableHeaderRowCount = 0x4104,
        TableBorderCollapse = 0x4105,

        // table cell properties
        TableCellRowSpan = 0x4810,
        TableCellColumnSpan = 0x4811,

        TableCellTopPadding = 0x4812,
        TableCellBottomPadding = 0x4813,
        TableCellLeftPadding = 0x4814,
        TableCellRightPadding = 0x4815,

        TableCellTopBorder = 0x4816,
        TableCellBottomBorder = 0x4817,
        TableCellLeftBorder = 0x4818,
        TableCellRightBorder = 0x4819,

        TableCellTopBorderStyle = 0x481a,
        TableCellBottomBorderStyle = 0x481b,
        TableCellLeftBorderStyle = 0x481c,
        TableCellRightBorderStyle = 0x481d,

        TableCellTopBorderBrush = 0x481e,
        TableCellBottomBorderBrush = 0x481f,
        TableCellLeftBorderBrush = 0x4820,
        TableCellRightBorderBrush = 0x4821,

        // image properties
        ImageName = 0x5000,
        ImageTitle = 0x5001,
        ImageAltText = 0x5002,
        ImageWidth = 0x5010,
        ImageHeight = 0x5011,
        ImageQuality = 0x5014,

        // selection properties
        FullWidthSelection = 0x06000,

        // page break properties
        PageBreakPolicy = 0x7000,

        // --
        UserProperty = 0x100000,
    }

    /// This enum describes what kind of `QTextObject` this format is associated with.
    #[repr(i32)]
    #[derive(Debug)]
    enum QTextFormatObjectTypes {
        NoObject,
        ImageObject,
        TableObject,
        TableCellObject,
        /// The first object that can be used for application-specific purposes.
        UserObject = 0x1000,
    }

    /// This enum describes how page breaking is performed when printing. It maps to the corresponding css properties.
    #[repr(i32)]
    enum QTextFormatPageBreakFlag {
        /// The page break is determined automatically depending on the available space on the current page
        PageBreak_Auto = 0,
        /// The page is always broken before the paragraph/table
        PageBreak_AlwaysBefore = 0x001,
        /// A new page is always started after the paragraph/table
        PageBreak_AlwaysAfter = 0x010,
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

        include!("smushclient-qt-lib/qbrush.h");
        type QBrush = crate::QBrush;
        include!("smushclient-qt-lib/qtextlength.h");
        type QTextLength = crate::QTextLength;
        include!("smushclient-qt-lib/qlist.h");
        type QList_QTextLength = cxx_qt_lib::QList<QTextLength>;
        include!("smushclient-qt-lib/qmap.h");
        type QMap_i32_QVariant = cxx_qt_lib::QMap<crate::QMapPair_i32_QVariant>;
    }

    #[namespace = "Qt"]
    extern "C++" {
        include!("cxx-qt-lib/qt.h");
        type LayoutDirection = cxx_qt_lib::LayoutDirection;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qtextformat.h");
        type QFontWeight;
        type QTextFormatFormatType;
        type QTextFormatProperty;
        type QTextFormatObjectTypes;
        type QTextFormatPageBreakFlag;
        type QTextFormatPageBreakFlags = super::QTextFormatPageBreakFlags;
    }

    unsafe extern "C++" {
        type QTextFormat = super::QTextFormat;

        #[doc(hidden)]
        #[rust_name = "bool_property_int"]
        fn boolProperty(&self, property_id: i32) -> bool;

        #[doc(hidden)]
        #[rust_name = "brush_property_int"]
        fn brushProperty(&self, property_id: i32) -> QBrush;

        #[doc(hidden)]
        #[rust_name = "clear_property_int"]
        fn clearProperty(&mut self, property_id: i32);

        #[doc(hidden)]
        #[rust_name = "color_property_int"]
        fn colorProperty(&self, property_id: i32) -> QColor;

        #[doc(hidden)]
        #[rust_name = "double_property_int"]
        fn doubleProperty(&self, property_id: i32) -> qreal;

        #[doc(hidden)]
        #[rust_name = "has_property_int"]
        fn hasProperty(&self, property_id: i32) -> bool;

        #[doc(hidden)]
        #[rust_name = "int_property_int"]
        fn intProperty(&self, property_id: i32) -> i32;

        /// Returns `true` if this text format is a `BlockFormat`; otherwise returns `false`.
        #[rust_name = "is_block_format"]
        fn isBlockFormat(&self) -> bool;

        /// Returns `true` if this text format is a `CharFormat`; otherwise returns `false`.
        #[rust_name = "is_char_format"]
        fn isCharFormat(&self) -> bool;

        /// Returns `true` if the format does not store any properties; `false` otherwise.
        #[rust_name = "is_empty"]
        fn isEmpty(&self) -> bool;

        /// Returns `true` if this text format is a `FrameFormat`; otherwise returns `false`.
        #[rust_name = "is_frame_format"]
        fn isFrameFormat(&self) -> bool;

        /// Returns `true` if this text format is an image format; otherwise returns `false`.
        #[rust_name = "is_image_format"]
        fn isImageFormat(&self) -> bool;

        /// Returns `true` if this text format is a `ListFormat`; otherwise returns `false`.
        #[rust_name = "is_list_format"]
        fn isListFormat(&self) -> bool;

        /// Returns `true` if this text format is a `TableCellFormat`; otherwise returns `false`.
        #[rust_name = "is_table_cell_format"]
        fn isTableCellFormat(&self) -> bool;

        /// Returns `true` if the format is valid (i.e. is not [`QTextFormatFormatType::InvalidFormat`]); otherwise returns `false`.
        #[rust_name = "is_valid"]
        fn isValid(&self) -> bool;

        #[doc(hidden)]
        #[rust_name = "length_property_int"]
        fn lengthProperty(&self, property_id: i32) -> QTextLength;

        #[doc(hidden)]
        #[rust_name = "length_vector_property_int"]
        fn lengthVectorProperty(&self, property_id: i32) -> QList_QTextLength;

        /// Merges the `other` format with this format; where there are conflicts the `other` format takes precedence.
        fn merge(&mut self, other: &QTextFormat);

        #[doc(hidden)]
        #[rust_name = "object_index_or_negative"]
        fn objectIndex(&self) -> i32;

        #[doc(hidden)]
        #[rust_name = "object_type_int"]
        fn objectType(&self) -> i32;

        #[doc(hidden)]
        #[rust_name = "pen_property_int"]
        fn penProperty(&self, property_id: i32) -> QPen;

        /// Returns a map with all properties of this text format.
        fn properties(&self) -> QMap_i32_QVariant;

        #[doc(hidden)]
        #[rust_name = "property_int"]
        fn property(&self, property_id: i32) -> QVariant;

        /// Returns the number of properties stored in the format.
        #[rust_name = "property_count"]
        fn propertyCount(&self) -> i32;

        /// Sets the format object's object `index`.
        #[rust_name = "set_object_index"]
        fn setObjectIndex(&mut self, index: i32);

        #[doc(hidden)]
        #[rust_name = "set_object_type_int"]
        fn setObjectType(&mut self, object_type: i32);

        #[doc(hidden)]
        #[rust_name = "set_property_int"]
        fn setProperty(&mut self, property_id: i32, value: &QVariant);

        #[doc(hidden)]
        #[rust_name = "set_length_vector_property_int"]
        fn setProperty(&mut self, property_id: i32, value: &QList_QTextLength);

        #[doc(hidden)]
        #[rust_name = "string_property_int"]
        fn stringProperty(&self, property_id: i32) -> QString;

        /// Swaps this text format with `other`. This operation is very fast and never fails.
        fn swap(&mut self, other: &mut QTextFormat);

        #[doc(hidden)]
        #[cxx_name = "type"]
        fn type_int(&self) -> i32;
    }

    #[namespace = "rust::smushclientqtlib1"]
    unsafe extern "C++" {}

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qtextformat_drop"]
        fn drop(config: &mut QTextFormat);

        #[rust_name = "qtextformat_init_default"]
        fn construct() -> QTextFormat;
        #[rust_name = "qtextformat_clone"]
        fn construct(other: &QTextFormat) -> QTextFormat;
        #[rust_name = "qtextformat_init_type"]
        fn construct(format_type: i32) -> QTextFormat;

        #[rust_name = "qtextformat_eq"]
        fn operatorEq(a: &QTextFormat, b: &QTextFormat) -> bool;

        #[rust_name = "qtextformat_to_debug_qstring"]
        fn toDebugQString(value: &QTextFormat) -> QString;
    }
}

pub use ffi::{
    QTextFormatFormatType, QTextFormatObjectTypes, QTextFormatPageBreakFlag, QTextFormatProperty,
};

use crate::{QBrush, QTextCharFormat, QTextLength};

/// [`QFlags`] of [`QTextFormatPageBreakFlag`].
pub type QTextFormatPageBreakFlags = QFlags<QTextFormatPageBreakFlag>;
unsafe_impl_qflag!(QTextFormatPageBreakFlag, "QTextFormatPageBreakFlags");

/// The `QTextFormat` class provides formatting information for a `QTextDocument`.
///
/// Qt Documentation: [QTextFormat](https://doc.qt.io/qt-6/qtextformat.html#details)
#[repr(C)]
pub struct QTextFormat {
    _space: MaybeUninit<usize>,
    format_type: i32,
}

impl Clone for QTextFormat {
    fn clone(&self) -> Self {
        ffi::qtextformat_clone(self)
    }
}

impl Default for QTextFormat {
    /// Default constructs a `QTextFormat` object.
    fn default() -> Self {
        ffi::qtextformat_init_default()
    }
}

impl Drop for QTextFormat {
    fn drop(&mut self) {
        ffi::qtextformat_drop(self);
    }
}

impl PartialEq for QTextFormat {
    fn eq(&self, other: &Self) -> bool {
        ffi::qtextformat_eq(self, other)
    }
}

impl Eq for QTextFormat {}

impl fmt::Debug for QTextFormat {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        ffi::qtextformat_to_debug_qstring(self).fmt(f)
    }
}

impl From<QTextFormatFormatType> for QTextFormat {
    fn from(value: QTextFormatFormatType) -> Self {
        ffi::qtextformat_init_type(value.repr)
    }
}

impl QTextFormat {
    /// Returns the property specified by the given `property_id`.
    pub fn property(&self, property_id: QTextFormatProperty) -> QVariant {
        self.property_int(property_id.repr)
    }

    pub fn set_property<T>(&mut self, property_id: QTextFormatProperty, value: T)
    where
        T: Into<QVariant>,
    {
        self.set_property_int(property_id.repr, &value.into());
    }

    pub fn clear_property(&mut self, property_id: QTextFormatProperty) {
        self.clear_property_int(property_id.repr);
    }

    /// Returns the value of the property specified by `property_id`. If the property isn't of `QMetaTypeType::Bool` type, `false` is returned instead.
    pub fn bool_property(&self, property_id: QTextFormatProperty) -> bool {
        self.bool_property_int(property_id.repr)
    }

    /// Returns the value of the property given by `property_id`; if the property isn't of `QMetaTypeType::QBrush` type, [`BrushStyle::NoBrush`](crate::BrushStyle::NoBrush) is returned instead.
    pub fn brush_property(&self, property_id: QTextFormatProperty) -> QBrush {
        self.brush_property_int(property_id.repr)
    }

    /// Returns the value of the property given by `property_id`; if the property isn't of `QMetaTypeType::QColor` type, an invalid color is returned instead.
    pub fn color_property(&self, property_id: QTextFormatProperty) -> QColor {
        self.color_property_int(property_id.repr)
    }

    /// Returns the value of the property specified by `property_id`. If the property isn't of `QMetaTypeType::Double` or `QMetaTypeType::Float` type, 0 is returned instead.
    pub fn double_property(&self, property_id: QTextFormatProperty) -> f64 {
        self.double_property_int(property_id.repr).into()
    }

    /// Returns `true` if the text format has a property with the given `property_id`; otherwise returns `false`.
    pub fn has_property(&self, property_id: QTextFormatProperty) -> bool {
        self.has_property_int(property_id.repr)
    }

    /// Returns the value of the property specified by `property_id`. If the property is not of `QMetaTypeType::Integer` type, 0 is returned instead.
    pub fn int_property(&self, property_id: QTextFormatProperty) -> i32 {
        self.int_property_int(property_id.repr)
    }

    /// Returns the value of the property given by `property_id`.
    pub fn length_property(&self, property_id: QTextFormatProperty) -> QTextLength {
        self.length_property_int(property_id.repr)
    }

    /// Returns the value of the property given by `property_id`. If the property isn't of `LengthVector` type, an empty list is returned instead.
    pub fn length_vector_property(&self, property_id: QTextFormatProperty) -> QList<QTextLength> {
        self.length_vector_property_int(property_id.repr)
    }

    /// Returns the value of the property given by `property_id`; if the property isn't of `QMetaTypeType::QPen`] type, [`PenStyle::NoPen`](cxx_qt_lib::PenStyle::NoPen) is returned instead.
    pub fn pen_property(&self, property_id: QTextFormatProperty) -> QPen {
        self.pen_property_int(property_id.repr)
    }

    /// Returns the value of the property given by `property_id`; if the property isn't of [`QMetaTypeType::QString`] type, an empty string is returned instead.
    pub fn string_property(&self, property_id: QTextFormatProperty) -> QString {
        self.string_property_int(property_id.repr)
    }

    /// Sets the property specified by the `property_id` to the given `value`.
    pub fn set_length_vector_property(
        &mut self,
        property_id: QTextFormatProperty,
        value: &QList<QTextLength>,
    ) {
        self.set_length_vector_property_int(property_id.repr, value);
    }

    /// Returns this format as a character format.
    /// Equivalent to `QTextCharFormat::from(&self)`
    pub fn to_char_format(&self) -> QTextCharFormat {
        self.into()
    }

    /// Returns the type of this format.
    pub fn format_type(&self) -> QTextFormatFormatType {
        QTextFormatFormatType {
            repr: self.type_int(),
        }
    }

    /// Returns the brush used to paint the document's background.
    pub fn background(&self) -> QBrush {
        self.brush_property(QTextFormatProperty::BackgroundBrush)
    }

    /// Clears the brush used to paint the document's background. The default brush will be used.
    pub fn clear_background(&mut self) {
        self.clear_property(QTextFormatProperty::BackgroundBrush);
    }

    /// Clears the brush used to paint the document's foreground. The default brush will be used.
    pub fn clear_foreground(&mut self) {
        self.clear_property(QTextFormatProperty::ForegroundBrush);
    }

    /// Returns the brush used to render foreground details, such as text, frame outlines, and table borders.
    pub fn foreground(&self) -> QBrush {
        self.brush_property(QTextFormatProperty::ForegroundBrush)
    }

    /// Returns the document's layout direction.
    pub fn layout_direction(&self) -> LayoutDirection {
        LayoutDirection {
            repr: self.int_property(QTextFormatProperty::LayoutDirection),
        }
    }

    /// Returns the index of the format object, or `None` if the format object is invalid.
    pub fn object_index(&self) -> Option<i32> {
        let index = self.object_index_or_negative();
        if index == -1 { None } else { Some(index) }
    }

    /// Returns the text format's object type.
    pub fn object_type(&self) -> QTextFormatObjectTypes {
        QTextFormatObjectTypes {
            repr: self.object_type_int(),
        }
    }

    /// Sets the brush use to paint the document's background to the `brush` specified.
    pub fn set_background(&mut self, brush: &QBrush) {
        self.set_property(QTextFormatProperty::BackgroundBrush, brush);
    }

    /// Sets the brush use to paint the document's foreground to the `brush` specified.
    pub fn set_foreground(&mut self, brush: &QBrush) {
        self.set_property(QTextFormatProperty::ForegroundBrush, brush);
    }

    /// Sets the document's layout direction to the specified `direction`.
    pub fn set_layout_direction(&mut self, direction: LayoutDirection) {
        self.set_property(QTextFormatProperty::LayoutDirection, &direction.repr);
    }

    /// Sets the text format's object type to `object_type`.
    pub fn set_object_type(&mut self, object_type: QTextFormatObjectTypes) {
        self.set_object_type_int(object_type.repr);
    }
}

impl QTextFormatFormatType {
    /// Represents a user-defined format. Equivalent to [`Self::UserFormat`]`+ value`.
    pub const fn user(value: i32) -> Self {
        Self {
            repr: Self::UserFormat.repr + value,
        }
    }
}

impl QTextFormatProperty {
    /// Represents a user-defined property. Equivalent to [`Self::UserProperty`]`+ value`.
    pub const fn user(value: i32) -> Self {
        Self {
            repr: Self::UserProperty.repr + value,
        }
    }
}

impl QTextFormatObjectTypes {
    /// Represents a user-defined object type. Equivalent to [`Self::UserObject`]`+ value`.
    pub const fn user(value: i32) -> Self {
        Self {
            repr: Self::UserObject.repr + value,
        }
    }
}

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QTextFormat {
    type Id = type_id!("QTextFormat");
    type Kind = cxx::kind::Trivial;
}
