//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("smushclient-qt-lib/qtextlength.h");
        type QTextLength = crate::QTextLength;

        include!("smushclient-qt-lib/qlist_qtextlength.h");
        type QList_QTextLength = cxx_qt_lib::QList<QTextLength>;
    }

    #[namespace = "rust::smushclientqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_clear_QTextLength"]
        fn qlistClear(list: &mut QList_QTextLength);
        #[rust_name = "qlist_contains_QTextLength"]
        fn qlistContains(list: &QList_QTextLength, _: &QTextLength) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qlist_clone_QTextLength"]
        fn construct(_: &QList_QTextLength) -> QList_QTextLength;
        #[rust_name = "qlist_default_QTextLength"]
        fn construct() -> QList_QTextLength;
        #[rust_name = "qlist_drop_QTextLength"]
        fn drop(_: &mut QList_QTextLength);
    }

    #[namespace = "rust::cxxqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_reserve_QTextLength"]
        fn qlistReserve(_: &mut QList_QTextLength, size: isize);
        #[rust_name = "qlist_append_QTextLength"]
        fn qlistAppend(_: &mut QList_QTextLength, _: &QTextLength);
        #[rust_name = "qlist_get_unchecked_QTextLength"]
        unsafe fn qlistGetUnchecked(set: &QList_QTextLength, pos: isize) -> &QTextLength;
        #[rust_name = "qlist_index_of_QTextLength"]
        fn qlistIndexOf(_: &QList_QTextLength, _: &QTextLength) -> isize;
        #[rust_name = "qlist_insert_QTextLength"]
        fn qlistInsert(_: &mut QList_QTextLength, _: isize, _: &QTextLength);
        #[rust_name = "qlist_remove_QTextLength"]
        fn qlistRemove(_: &mut QList_QTextLength, _: isize);
        #[rust_name = "qlist_len_QTextLength"]
        fn qlistLen(_: &QList_QTextLength) -> isize;
    }
}

pub(crate) fn clear(v: &mut ffi::QList_QTextLength) {
    ffi::qlist_clear_QTextLength(v);
}

pub(crate) fn contains(v: &ffi::QList_QTextLength, item: &ffi::QTextLength) -> bool {
    ffi::qlist_contains_QTextLength(v, item)
}

pub(crate) fn reserve(v: &mut ffi::QList_QTextLength, size: isize) {
    ffi::qlist_reserve_QTextLength(v, size);
}

pub(crate) fn append(v: &mut ffi::QList_QTextLength, value: &ffi::QTextLength) {
    ffi::qlist_append_QTextLength(v, value);
}

pub(crate) fn clone(s: &ffi::QList_QTextLength) -> ffi::QList_QTextLength {
    ffi::qlist_clone_QTextLength(s)
}

pub(crate) fn default() -> ffi::QList_QTextLength {
    ffi::qlist_default_QTextLength()
}

pub(crate) fn drop(s: &mut ffi::QList_QTextLength) {
    ffi::qlist_drop_QTextLength(s);
}

pub(crate) unsafe fn get_unchecked(s: &ffi::QList_QTextLength, pos: isize) -> &ffi::QTextLength {
    unsafe { ffi::qlist_get_unchecked_QTextLength(s, pos) }
}

pub(crate) fn index_of(v: &ffi::QList_QTextLength, value: &ffi::QTextLength) -> isize {
    ffi::qlist_index_of_QTextLength(v, value)
}

pub(crate) fn insert(s: &mut ffi::QList_QTextLength, pos: isize, value: &ffi::QTextLength) {
    ffi::qlist_insert_QTextLength(s, pos, value);
}

pub(crate) fn len(s: &ffi::QList_QTextLength) -> isize {
    ffi::qlist_len_QTextLength(s)
}

pub(crate) fn remove(s: &mut ffi::QList_QTextLength, pos: isize) {
    ffi::qlist_remove_QTextLength(s, pos);
}
