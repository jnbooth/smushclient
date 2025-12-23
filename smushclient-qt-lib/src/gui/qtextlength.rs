use cxx::{ExternType, type_id};

#[cxx::bridge]
mod ffi {
    /// This enum describes the different types a length object can have.
    #[repr(i32)]
    #[derive(Debug)]
    enum QTextLengthType {
        /// The width of the object is variable
        VariableLength,
        /// The width of the object is fixed
        FixedLength,
        /// The width of the object is in percentage of the maximum width
        PercentageLength,
    }

    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;
        include!("cxx-qt-lib/qtypes.h");
        type qreal = cxx_qt_lib::qreal;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qtextlength.h");
        type QTextLengthType;
    }

    unsafe extern "C++" {
        type QTextLength = super::QTextLength;

        #[doc(hidden)]
        #[rust_name = "raw_value_qreal"]
        fn rawValue(&self) -> qreal;

        /// Returns the type of this length object.
        #[cxx_name = "type"]
        fn length_type(&self) -> QTextLengthType;

        #[doc(hidden)]
        #[rust_name = "value_qreal"]
        fn value(&self, maximum_length: qreal) -> qreal;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qtextlength_init_default"]
        fn construct() -> QTextLength;
        #[rust_name = "qtextlength_new"]
        fn construct(length_type: QTextLengthType, maximum_length: qreal) -> QTextLength;
    }
}

pub use ffi::QTextLengthType;

/// The `QTextLength` encapsulates the different types of length used in a `QTextDocument`.
///
/// Qt Documentation: [QTextLength](https://doc.qt.io/qt-6/qtextlength.html#details)
#[repr(C)]
#[derive(Debug, Clone, PartialEq)]
pub struct QTextLength {
    length_type: i32,
    fixed_value_or_percentage: f64,
}

impl Default for QTextLength {
    /// Constructs a new length object which represents a variable size.
    fn default() -> Self {
        ffi::qtextlength_init_default()
    }
}

impl QTextLength {
    /// Constructs a new length object of the given `length_type` and `value`.
    pub fn new(length_type: QTextLengthType, value: f64) -> Self {
        ffi::qtextlength_new(length_type, value.into())
    }

    /// Returns the constraint value that is specific for the type of the length. If the length is [`QTextLengthType::PercentageLength`] then the raw value is in percent, in the range of 0 to 100. If the length is [`QTextLengthType::FixedLength`] then that fixed amount is returned. For variable lengths, zero is returned.
    pub fn raw_value(&self) -> f64 {
        self.raw_value_qreal().into()
    }

    /// Returns the effective length, constrained by the type of the length object and the specified `maximum_length`.
    pub fn value(&self, maximum_length: f64) -> f64 {
        self.value_qreal(maximum_length.into()).into()
    }
}

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QTextLength {
    type Id = type_id!("QTextLength");
    type Kind = cxx::kind::Trivial;
}
