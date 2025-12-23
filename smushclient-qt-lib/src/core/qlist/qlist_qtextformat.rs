//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("smushclient-qt-lib/qtextformat.h");
        type QTextFormat = crate::QTextFormat;

        include!("smushclient-qt-lib/qlist_qtextformat.h");
        type QList_QTextFormat = cxx_qt_lib::QList<QTextFormat>;
    }

    #[namespace = "rust::smushclientqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_clear_QTextFormat"]
        fn qlistClear(list: &mut QList_QTextFormat);
        #[rust_name = "qlist_contains_QTextFormat"]
        fn qlistContains(list: &QList_QTextFormat, _: &QTextFormat) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qlist_clone_QTextFormat"]
        fn construct(_: &QList_QTextFormat) -> QList_QTextFormat;
        #[rust_name = "qlist_default_QTextFormat"]
        fn construct() -> QList_QTextFormat;
        #[rust_name = "qlist_drop_QTextFormat"]
        fn drop(_: &mut QList_QTextFormat);
    }

    #[namespace = "rust::cxxqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_reserve_QTextFormat"]
        fn qlistReserve(_: &mut QList_QTextFormat, size: isize);
        #[rust_name = "qlist_append_QTextFormat"]
        fn qlistAppend(_: &mut QList_QTextFormat, _: &QTextFormat);
        #[rust_name = "qlist_get_unchecked_QTextFormat"]
        unsafe fn qlistGetUnchecked(set: &QList_QTextFormat, pos: isize) -> &QTextFormat;
        #[rust_name = "qlist_index_of_QTextFormat"]
        fn qlistIndexOf(_: &QList_QTextFormat, _: &QTextFormat) -> isize;
        #[rust_name = "qlist_insert_QTextFormat"]
        fn qlistInsert(_: &mut QList_QTextFormat, _: isize, _: &QTextFormat);
        #[rust_name = "qlist_remove_QTextFormat"]
        fn qlistRemove(_: &mut QList_QTextFormat, _: isize);
        #[rust_name = "qlist_len_QTextFormat"]
        fn qlistLen(_: &QList_QTextFormat) -> isize;
    }
}

pub(crate) fn clear(v: &mut ffi::QList_QTextFormat) {
    ffi::qlist_clear_QTextFormat(v);
}

pub(crate) fn contains(v: &ffi::QList_QTextFormat, item: &ffi::QTextFormat) -> bool {
    ffi::qlist_contains_QTextFormat(v, item)
}

pub(crate) fn reserve(v: &mut ffi::QList_QTextFormat, size: isize) {
    ffi::qlist_reserve_QTextFormat(v, size);
}

pub(crate) fn append(v: &mut ffi::QList_QTextFormat, value: &ffi::QTextFormat) {
    ffi::qlist_append_QTextFormat(v, value);
}

pub(crate) fn clone(s: &ffi::QList_QTextFormat) -> ffi::QList_QTextFormat {
    ffi::qlist_clone_QTextFormat(s)
}

pub(crate) fn default() -> ffi::QList_QTextFormat {
    ffi::qlist_default_QTextFormat()
}

pub(crate) fn drop(s: &mut ffi::QList_QTextFormat) {
    ffi::qlist_drop_QTextFormat(s);
}

pub(crate) unsafe fn get_unchecked(s: &ffi::QList_QTextFormat, pos: isize) -> &ffi::QTextFormat {
    unsafe { ffi::qlist_get_unchecked_QTextFormat(s, pos) }
}

pub(crate) fn index_of(v: &ffi::QList_QTextFormat, value: &ffi::QTextFormat) -> isize {
    ffi::qlist_index_of_QTextFormat(v, value)
}

pub(crate) fn insert(s: &mut ffi::QList_QTextFormat, pos: isize, value: &ffi::QTextFormat) {
    ffi::qlist_insert_QTextFormat(s, pos, value);
}

pub(crate) fn len(s: &ffi::QList_QTextFormat) -> isize {
    ffi::qlist_len_QTextFormat(s)
}

pub(crate) fn remove(s: &mut ffi::QList_QTextFormat, pos: isize) {
    ffi::qlist_remove_QTextFormat(s, pos);
}
