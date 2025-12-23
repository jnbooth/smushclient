//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("smushclient-qt-lib/qbrush.h");
        type QBrush = crate::QBrush;

        include!("smushclient-qt-lib/qlist_qbrush.h");
        type QList_QBrush = cxx_qt_lib::QList<QBrush>;
    }

    #[namespace = "rust::smushclientqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_clear_QBrush"]
        fn qlistClear(list: &mut QList_QBrush);
        #[rust_name = "qlist_contains_QBrush"]
        fn qlistContains(list: &QList_QBrush, _: &QBrush) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qlist_clone_QBrush"]
        fn construct(_: &QList_QBrush) -> QList_QBrush;
        #[rust_name = "qlist_default_QBrush"]
        fn construct() -> QList_QBrush;
        #[rust_name = "qlist_drop_QBrush"]
        fn drop(_: &mut QList_QBrush);
    }

    #[namespace = "rust::cxxqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_reserve_QBrush"]
        fn qlistReserve(_: &mut QList_QBrush, size: isize);
        #[rust_name = "qlist_append_QBrush"]
        fn qlistAppend(_: &mut QList_QBrush, _: &QBrush);
        #[rust_name = "qlist_get_unchecked_QBrush"]
        unsafe fn qlistGetUnchecked(set: &QList_QBrush, pos: isize) -> &QBrush;
        #[rust_name = "qlist_index_of_QBrush"]
        fn qlistIndexOf(_: &QList_QBrush, _: &QBrush) -> isize;
        #[rust_name = "qlist_insert_QBrush"]
        fn qlistInsert(_: &mut QList_QBrush, _: isize, _: &QBrush);
        #[rust_name = "qlist_remove_QBrush"]
        fn qlistRemove(_: &mut QList_QBrush, _: isize);
        #[rust_name = "qlist_len_QBrush"]
        fn qlistLen(_: &QList_QBrush) -> isize;
    }
}

pub(crate) fn clear(v: &mut ffi::QList_QBrush) {
    ffi::qlist_clear_QBrush(v);
}

pub(crate) fn contains(v: &ffi::QList_QBrush, item: &ffi::QBrush) -> bool {
    ffi::qlist_contains_QBrush(v, item)
}

pub(crate) fn reserve(v: &mut ffi::QList_QBrush, size: isize) {
    ffi::qlist_reserve_QBrush(v, size);
}

pub(crate) fn append(v: &mut ffi::QList_QBrush, value: &ffi::QBrush) {
    ffi::qlist_append_QBrush(v, value);
}

pub(crate) fn clone(s: &ffi::QList_QBrush) -> ffi::QList_QBrush {
    ffi::qlist_clone_QBrush(s)
}

pub(crate) fn default() -> ffi::QList_QBrush {
    ffi::qlist_default_QBrush()
}

pub(crate) fn drop(s: &mut ffi::QList_QBrush) {
    ffi::qlist_drop_QBrush(s);
}

pub(crate) unsafe fn get_unchecked(s: &ffi::QList_QBrush, pos: isize) -> &ffi::QBrush {
    unsafe { ffi::qlist_get_unchecked_QBrush(s, pos) }
}

pub(crate) fn index_of(v: &ffi::QList_QBrush, value: &ffi::QBrush) -> isize {
    ffi::qlist_index_of_QBrush(v, value)
}

pub(crate) fn insert(s: &mut ffi::QList_QBrush, pos: isize, value: &ffi::QBrush) {
    ffi::qlist_insert_QBrush(s, pos, value);
}

pub(crate) fn len(s: &ffi::QList_QBrush) -> isize {
    ffi::qlist_len_QBrush(s)
}

pub(crate) fn remove(s: &mut ffi::QList_QBrush, pos: isize) {
    ffi::qlist_remove_QBrush(s, pos);
}
