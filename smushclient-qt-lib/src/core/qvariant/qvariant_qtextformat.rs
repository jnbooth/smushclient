//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qvariant/generate.sh

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;

        include!("smushclient-qt-lib/qvariant_qtextformat.h");
        type QTextFormat = crate::QTextFormat;
    }

    #[namespace = "rust::cxxqtlib1::qvariant"]
    unsafe extern "C++" {
        #[rust_name = "qvariant_can_convert_QTextFormat"]
        fn qvariantCanConvertQTextFormat(variant: &QVariant) -> bool;
        #[rust_name = "qvariant_construct_QTextFormat"]
        fn qvariantConstruct(value: &QTextFormat) -> QVariant;
        #[rust_name = "qvariant_value_or_default_QTextFormat"]
        fn qvariantValueOrDefault(variant: &QVariant) -> QTextFormat;
    }
}

pub(crate) fn can_convert(variant: &ffi::QVariant) -> bool {
    ffi::qvariant_can_convert_QTextFormat(variant)
}

pub(crate) fn construct(value: &ffi::QTextFormat) -> ffi::QVariant {
    ffi::qvariant_construct_QTextFormat(value)
}

pub(crate) fn value_or_default(variant: &ffi::QVariant) -> ffi::QTextFormat {
    ffi::qvariant_value_or_default_QTextFormat(variant)
}
