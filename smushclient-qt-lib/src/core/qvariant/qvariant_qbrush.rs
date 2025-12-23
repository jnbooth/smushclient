//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qvariant/generate.sh

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;

        include!("smushclient-qt-lib/qvariant_qbrush.h");
        type QBrush = crate::QBrush;
    }

    #[namespace = "rust::cxxqtlib1::qvariant"]
    unsafe extern "C++" {
        #[rust_name = "qvariant_can_convert_QBrush"]
        fn qvariantCanConvertQBrush(variant: &QVariant) -> bool;
        #[rust_name = "qvariant_construct_QBrush"]
        fn qvariantConstruct(value: &QBrush) -> QVariant;
        #[rust_name = "qvariant_value_or_default_QBrush"]
        fn qvariantValueOrDefault(variant: &QVariant) -> QBrush;
    }
}

pub(crate) fn can_convert(variant: &ffi::QVariant) -> bool {
    ffi::qvariant_can_convert_QBrush(variant)
}

pub(crate) fn construct(value: &ffi::QBrush) -> ffi::QVariant {
    ffi::qvariant_construct_QBrush(value)
}

pub(crate) fn value_or_default(variant: &ffi::QVariant) -> ffi::QBrush {
    ffi::qvariant_value_or_default_QBrush(variant)
}
