//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("smushclient-qt-lib/qtextcharformat.h");
        type QTextCharFormat = crate::QTextCharFormat;

        include!("smushclient-qt-lib/qlist_qtextcharformat.h");
        type QList_QTextCharFormat = cxx_qt_lib::QList<QTextCharFormat>;
    }

    #[namespace = "rust::smushclientqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_clear_QTextCharFormat"]
        fn qlistClear(list: &mut QList_QTextCharFormat);
        #[rust_name = "qlist_contains_QTextCharFormat"]
        fn qlistContains(list: &QList_QTextCharFormat, _: &QTextCharFormat) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qlist_clone_QTextCharFormat"]
        fn construct(_: &QList_QTextCharFormat) -> QList_QTextCharFormat;
        #[rust_name = "qlist_default_QTextCharFormat"]
        fn construct() -> QList_QTextCharFormat;
        #[rust_name = "qlist_drop_QTextCharFormat"]
        fn drop(_: &mut QList_QTextCharFormat);
    }

    #[namespace = "rust::cxxqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_reserve_QTextCharFormat"]
        fn qlistReserve(_: &mut QList_QTextCharFormat, size: isize);
        #[rust_name = "qlist_append_QTextCharFormat"]
        fn qlistAppend(_: &mut QList_QTextCharFormat, _: &QTextCharFormat);
        #[rust_name = "qlist_get_unchecked_QTextCharFormat"]
        unsafe fn qlistGetUnchecked(set: &QList_QTextCharFormat, pos: isize) -> &QTextCharFormat;
        #[rust_name = "qlist_index_of_QTextCharFormat"]
        fn qlistIndexOf(_: &QList_QTextCharFormat, _: &QTextCharFormat) -> isize;
        #[rust_name = "qlist_insert_QTextCharFormat"]
        fn qlistInsert(_: &mut QList_QTextCharFormat, _: isize, _: &QTextCharFormat);
        #[rust_name = "qlist_remove_QTextCharFormat"]
        fn qlistRemove(_: &mut QList_QTextCharFormat, _: isize);
        #[rust_name = "qlist_len_QTextCharFormat"]
        fn qlistLen(_: &QList_QTextCharFormat) -> isize;
    }
}

pub(crate) fn clear(v: &mut ffi::QList_QTextCharFormat) {
    ffi::qlist_clear_QTextCharFormat(v);
}

pub(crate) fn contains(v: &ffi::QList_QTextCharFormat, item: &ffi::QTextCharFormat) -> bool {
    ffi::qlist_contains_QTextCharFormat(v, item)
}

pub(crate) fn reserve(v: &mut ffi::QList_QTextCharFormat, size: isize) {
    ffi::qlist_reserve_QTextCharFormat(v, size);
}

pub(crate) fn append(v: &mut ffi::QList_QTextCharFormat, value: &ffi::QTextCharFormat) {
    ffi::qlist_append_QTextCharFormat(v, value);
}

pub(crate) fn clone(s: &ffi::QList_QTextCharFormat) -> ffi::QList_QTextCharFormat {
    ffi::qlist_clone_QTextCharFormat(s)
}

pub(crate) fn default() -> ffi::QList_QTextCharFormat {
    ffi::qlist_default_QTextCharFormat()
}

pub(crate) fn drop(s: &mut ffi::QList_QTextCharFormat) {
    ffi::qlist_drop_QTextCharFormat(s);
}

pub(crate) unsafe fn get_unchecked(
    s: &ffi::QList_QTextCharFormat,
    pos: isize,
) -> &ffi::QTextCharFormat {
    unsafe { ffi::qlist_get_unchecked_QTextCharFormat(s, pos) }
}

pub(crate) fn index_of(v: &ffi::QList_QTextCharFormat, value: &ffi::QTextCharFormat) -> isize {
    ffi::qlist_index_of_QTextCharFormat(v, value)
}

pub(crate) fn insert(s: &mut ffi::QList_QTextCharFormat, pos: isize, value: &ffi::QTextCharFormat) {
    ffi::qlist_insert_QTextCharFormat(s, pos, value);
}

pub(crate) fn len(s: &ffi::QList_QTextCharFormat) -> isize {
    ffi::qlist_len_QTextCharFormat(s)
}

pub(crate) fn remove(s: &mut ffi::QList_QTextCharFormat, pos: isize) {
    ffi::qlist_remove_QTextCharFormat(s, pos);
}
