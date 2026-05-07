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
    }

    #[namespace = "rust::cxxqtio1::qvariant"]
    unsafe extern "C++" {
        include!("cxx-qt-io/qvariant.h");

        #[rust_name = "qvariant_init_value_or_default_QBrush"]
        unsafe fn qvariantInitValueOrDefault(variant: &QVariant, uninit: *mut QBrush);
    }
}

pub(crate) fn can_convert(variant: &ffi::QVariant) -> bool {
    ffi::qvariant_can_convert_QBrush(variant)
}

pub(crate) fn construct(value: &ffi::QBrush) -> ffi::QVariant {
    ffi::qvariant_construct_QBrush(value)
}

pub(crate) fn value_or_default(variant: &ffi::QVariant) -> ffi::QBrush {
    let mut uninit = std::mem::MaybeUninit::uninit();
    // SAFETY: uninit.as_mut_ptr() is valid.
    unsafe { ffi::qvariant_init_value_or_default_QBrush(variant, uninit.as_mut_ptr()) };
    // SAFETY: ffi::qvariant_init_value_or_default_QBrush initializes the passed pointer in-place.
    unsafe { uninit.assume_init() }
}

#[cfg(test)]
mod tests {
    #[test]
    fn can_convert() {
        assert!(!super::can_convert(&cxx_qt_lib::QVariant::default()));
    }
}
