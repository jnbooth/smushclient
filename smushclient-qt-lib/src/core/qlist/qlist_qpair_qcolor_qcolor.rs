//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("smushclient-qt-lib/qpair_qcolor_qcolor.h");
        type QPair_QColor_QColor = crate::QPair<cxx_qt_lib::QColor, cxx_qt_lib::QColor>;

        include!("smushclient-qt-lib/qlist_qpair_qcolor_qcolor.h");
        type QList_QPair_QColor_QColor = cxx_qt_lib::QList<QPair_QColor_QColor>;
    }

    #[namespace = "rust::smushclientqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_clear_QPair_QColor_QColor"]
        fn qlistClear(list: &mut QList_QPair_QColor_QColor);
        #[rust_name = "qlist_contains_QPair_QColor_QColor"]
        fn qlistContains(list: &QList_QPair_QColor_QColor, _: &QPair_QColor_QColor) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qlist_clone_QPair_QColor_QColor"]
        fn construct(_: &QList_QPair_QColor_QColor) -> QList_QPair_QColor_QColor;
        #[rust_name = "qlist_default_QPair_QColor_QColor"]
        fn construct() -> QList_QPair_QColor_QColor;
        #[rust_name = "qlist_drop_QPair_QColor_QColor"]
        fn drop(_: &mut QList_QPair_QColor_QColor);
    }

    #[namespace = "rust::cxxqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_reserve_QPair_QColor_QColor"]
        fn qlistReserve(_: &mut QList_QPair_QColor_QColor, size: isize);
        #[rust_name = "qlist_append_QPair_QColor_QColor"]
        fn qlistAppend(_: &mut QList_QPair_QColor_QColor, _: &QPair_QColor_QColor);
        #[rust_name = "qlist_get_unchecked_QPair_QColor_QColor"]
        unsafe fn qlistGetUnchecked(
            set: &QList_QPair_QColor_QColor,
            pos: isize,
        ) -> &QPair_QColor_QColor;
        #[rust_name = "qlist_index_of_QPair_QColor_QColor"]
        fn qlistIndexOf(_: &QList_QPair_QColor_QColor, _: &QPair_QColor_QColor) -> isize;
        #[rust_name = "qlist_insert_QPair_QColor_QColor"]
        fn qlistInsert(_: &mut QList_QPair_QColor_QColor, _: isize, _: &QPair_QColor_QColor);
        #[rust_name = "qlist_remove_QPair_QColor_QColor"]
        fn qlistRemove(_: &mut QList_QPair_QColor_QColor, _: isize);
        #[rust_name = "qlist_len_QPair_QColor_QColor"]
        fn qlistLen(_: &QList_QPair_QColor_QColor) -> isize;
    }
}

pub(crate) fn clear(v: &mut ffi::QList_QPair_QColor_QColor) {
    ffi::qlist_clear_QPair_QColor_QColor(v);
}

pub(crate) fn contains(
    v: &ffi::QList_QPair_QColor_QColor,
    item: &ffi::QPair_QColor_QColor,
) -> bool {
    ffi::qlist_contains_QPair_QColor_QColor(v, item)
}

pub(crate) fn reserve(v: &mut ffi::QList_QPair_QColor_QColor, size: isize) {
    ffi::qlist_reserve_QPair_QColor_QColor(v, size);
}

pub(crate) fn append(v: &mut ffi::QList_QPair_QColor_QColor, value: &ffi::QPair_QColor_QColor) {
    ffi::qlist_append_QPair_QColor_QColor(v, value);
}

pub(crate) fn clone(s: &ffi::QList_QPair_QColor_QColor) -> ffi::QList_QPair_QColor_QColor {
    ffi::qlist_clone_QPair_QColor_QColor(s)
}

pub(crate) fn default() -> ffi::QList_QPair_QColor_QColor {
    ffi::qlist_default_QPair_QColor_QColor()
}

pub(crate) fn drop(s: &mut ffi::QList_QPair_QColor_QColor) {
    ffi::qlist_drop_QPair_QColor_QColor(s);
}

pub(crate) unsafe fn get_unchecked(
    s: &ffi::QList_QPair_QColor_QColor,
    pos: isize,
) -> &ffi::QPair_QColor_QColor {
    unsafe { ffi::qlist_get_unchecked_QPair_QColor_QColor(s, pos) }
}

pub(crate) fn index_of(
    v: &ffi::QList_QPair_QColor_QColor,
    value: &ffi::QPair_QColor_QColor,
) -> isize {
    ffi::qlist_index_of_QPair_QColor_QColor(v, value)
}

pub(crate) fn insert(
    s: &mut ffi::QList_QPair_QColor_QColor,
    pos: isize,
    value: &ffi::QPair_QColor_QColor,
) {
    ffi::qlist_insert_QPair_QColor_QColor(s, pos, value);
}

pub(crate) fn len(s: &ffi::QList_QPair_QColor_QColor) -> isize {
    ffi::qlist_len_QPair_QColor_QColor(s)
}

pub(crate) fn remove(s: &mut ffi::QList_QPair_QColor_QColor, pos: isize) {
    ffi::qlist_remove_QPair_QColor_QColor(s, pos);
}
