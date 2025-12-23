//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qvariant/generate.sh

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;

        include!("smushclient-qt-lib/qvariant_qtextlength.h");
        type QTextLength = crate::QTextLength;
    }

    #[namespace = "rust::cxxqtlib1::qvariant"]
    unsafe extern "C++" {
        #[rust_name = "qvariant_can_convert_QTextLength"]
        fn qvariantCanConvertQTextLength(variant: &QVariant) -> bool;
        #[rust_name = "qvariant_construct_QTextLength"]
        fn qvariantConstruct(value: &QTextLength) -> QVariant;
        #[rust_name = "qvariant_value_or_default_QTextLength"]
        fn qvariantValueOrDefault(variant: &QVariant) -> QTextLength;
    }
}

pub(crate) fn can_convert(variant: &ffi::QVariant) -> bool {
    ffi::qvariant_can_convert_QTextLength(variant)
}

pub(crate) fn construct(value: &ffi::QTextLength) -> ffi::QVariant {
    ffi::qvariant_construct_QTextLength(value)
}

pub(crate) fn value_or_default(variant: &ffi::QVariant) -> ffi::QTextLength {
    ffi::qvariant_value_or_default_QTextLength(variant)
}
