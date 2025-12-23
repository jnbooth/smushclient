use cxx::{ExternType, type_id};

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;

        include!("smushclient-qt-lib/qmap_i32_qvariant.h");
        type QMap_i32_QVariant = cxx_qt_lib::QMap<super::QMapPair_i32_QVariant>;
    }

    #[namespace = "rust::smushclientqtlib1::qmap"]
    unsafe extern "C++" {
        #[rust_name = "qmap_clear_i32_QVariant"]
        fn qmapClear(map: &mut QMap_i32_QVariant);
        #[rust_name = "qmap_contains_i32_QVariant"]
        fn qmapContains(list: &QMap_i32_QVariant, _: &i32) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qmap_clone_i32_QVariant"]
        fn construct(_: &QMap_i32_QVariant) -> QMap_i32_QVariant;
        #[rust_name = "qmap_default_i32_QVariant"]
        fn construct() -> QMap_i32_QVariant;
        #[rust_name = "qmap_drop_i32_QVariant"]
        fn drop(_: &mut QMap_i32_QVariant);
    }

    #[namespace = "rust::cxxqtlib1::qmap"]
    unsafe extern "C++" {
        #[rust_name = "qmap_get_or_default_i32_QVariant"]
        fn qmapGetOrDefault(_: &QMap_i32_QVariant, key: &i32) -> QVariant;
        #[rust_name = "qmap_get_unchecked_key_i32_QVariant"]
        unsafe fn qmapGetUncheckedKey<'a>(_: &'a QMap_i32_QVariant, pos: isize) -> &'a i32;
        #[rust_name = "qmap_get_unchecked_value_i32_QVariant"]
        unsafe fn qmapGetUncheckedValue(_: &QMap_i32_QVariant, pos: isize) -> &QVariant;
        #[rust_name = "qmap_insert_i32_QVariant"]
        fn qmapInsert(_: &mut QMap_i32_QVariant, key: &i32, value: &QVariant);
        #[rust_name = "qmap_len_i32_QVariant"]
        fn qmapLen(_: &QMap_i32_QVariant) -> isize;
        #[rust_name = "qmap_remove_i32_QVariant"]
        fn qmapRemove(_: &mut QMap_i32_QVariant, key: &i32) -> bool;
    }
}

pub(crate) fn clear(v: &mut ffi::QMap_i32_QVariant) {
    ffi::qmap_clear_i32_QVariant(v);
}

pub(crate) fn contains(v: &ffi::QMap_i32_QVariant, item: &i32) -> bool {
    ffi::qmap_contains_i32_QVariant(v, item)
}

pub(crate) fn clone(map: &ffi::QMap_i32_QVariant) -> ffi::QMap_i32_QVariant {
    ffi::qmap_clone_i32_QVariant(map)
}

pub(crate) fn default() -> ffi::QMap_i32_QVariant {
    ffi::qmap_default_i32_QVariant()
}

pub(crate) fn drop(map: &mut ffi::QMap_i32_QVariant) {
    ffi::qmap_drop_i32_QVariant(map);
}

pub(crate) fn get_or_default(map: &ffi::QMap_i32_QVariant, key: &i32) -> ffi::QVariant {
    ffi::qmap_get_or_default_i32_QVariant(map, key)
}

pub(crate) unsafe fn get_unchecked_key(map: &ffi::QMap_i32_QVariant, pos: isize) -> &i32 {
    unsafe { ffi::qmap_get_unchecked_key_i32_QVariant(map, pos) }
}

pub(crate) unsafe fn get_unchecked_value(
    map: &ffi::QMap_i32_QVariant,
    pos: isize,
) -> &ffi::QVariant {
    unsafe { ffi::qmap_get_unchecked_value_i32_QVariant(map, pos) }
}

pub(crate) fn insert(map: &mut ffi::QMap_i32_QVariant, key: &i32, value: &ffi::QVariant) {
    ffi::qmap_insert_i32_QVariant(map, key, value);
}

pub(crate) fn len(map: &ffi::QMap_i32_QVariant) -> isize {
    ffi::qmap_len_i32_QVariant(map)
}

pub(crate) fn remove(map: &mut ffi::QMap_i32_QVariant, key: &i32) -> bool {
    ffi::qmap_remove_i32_QVariant(map, key)
}

pub struct QMapPair_i32_QVariant;

// SAFETY: Static checks on the C++ side to ensure the size is the same.
unsafe impl ExternType for QMapPair_i32_QVariant {
    type Id = type_id!("QMapPair_i32_QVariant");
    type Kind = cxx::kind::Trivial;
}
