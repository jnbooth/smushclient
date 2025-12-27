#!/usr/bin/env bash

set -e

SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

function generate_bridge() {
    local LOWER
    LOWER=$(echo "$1" | tr '[:upper:]' '[:lower:]')
    local IMPORT="$2"

    if [[ -z "$IMPORT" ]]; then
        IMPORT="$1"
    fi

    tee "$SCRIPTPATH/../../../include/core/qlist/qlist_$LOWER.h" <<EOF
//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#pragma once
#include "qlist_private.h"
#include <smushclient-qt-lib/$LOWER.h>

using QList_$1 = QList<$1>;
EOF
    clang-format -i "$SCRIPTPATH/../../../include/core/qlist/qlist_$LOWER.h"

    tee "$SCRIPTPATH/qlist_$LOWER.rs" <<EOF
//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qlist/generate.sh

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("smushclient-qt-lib/$LOWER.h");
        type $1 = crate::$IMPORT;

        include!("smushclient-qt-lib/qlist_$LOWER.h");
        type QList_$1 = cxx_qt_lib::QList<$1>;
    }

    #[namespace = "rust::smushclientqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_clear_$1"]
        fn qlistClear(list: &mut QList_$1);
        #[rust_name = "qlist_contains_$1"]
        fn qlistContains(
            list: &QList_$1,
            _: &$1,
        ) -> bool;
    }

    #[namespace = "rust::cxxqtlib1"]
    unsafe extern "C++" {
        include!("cxx-qt-lib/common.h");

        #[rust_name = "qlist_clone_$1"]
        fn construct(_: &QList_$1) -> QList_$1;
        #[rust_name = "qlist_default_$1"]
        fn construct() -> QList_$1;
        #[rust_name = "qlist_drop_$1"]
        fn drop(_: &mut QList_$1);
    }

    #[namespace = "rust::cxxqtlib1::qlist"]
    unsafe extern "C++" {
        #[rust_name = "qlist_reserve_$1"]
        fn qlistReserve(_: &mut QList_$1, size: isize);
        #[rust_name = "qlist_append_$1"]
        fn qlistAppend(_: &mut QList_$1, _: &$1);
        #[rust_name = "qlist_get_unchecked_$1"]
        unsafe fn qlistGetUnchecked(set: &QList_$1, pos: isize) -> &$1;
        #[rust_name = "qlist_index_of_$1"]
        fn qlistIndexOf(_: &QList_$1, _: &$1) -> isize;
        #[rust_name = "qlist_insert_$1"]
        fn qlistInsert(_: &mut QList_$1, _: isize, _: &$1);
        #[rust_name = "qlist_remove_$1"]
        fn qlistRemove(_: &mut QList_$1, _: isize);
        #[rust_name = "qlist_len_$1"]
        fn qlistLen(_: &QList_$1) -> isize;
    }
}

pub(crate) fn clear(v: &mut ffi::QList_$1) {
    ffi::qlist_clear_$1(v);
}

pub(crate) fn contains(
    v: &ffi::QList_$1,
    item: &ffi::$1,
) -> bool {
    ffi::qlist_contains_$1(v, item)
}

pub(crate) fn reserve(v: &mut ffi::QList_$1, size: isize) {
    ffi::qlist_reserve_$1(v, size);
}

pub(crate) fn append(v: &mut ffi::QList_$1, value: &ffi::$1) {
    ffi::qlist_append_$1(v, value);
}

pub(crate) fn clone(s: &ffi::QList_$1) -> ffi::QList_$1 {
    ffi::qlist_clone_$1(s)
}

pub(crate) fn default() -> ffi::QList_$1 {
    ffi::qlist_default_$1()
}

pub(crate) fn drop(s: &mut ffi::QList_$1) {
    ffi::qlist_drop_$1(s);
}

pub(crate) unsafe fn get_unchecked(s: &ffi::QList_$1, pos: isize) -> &ffi::$1 {
    unsafe { ffi::qlist_get_unchecked_$1(s, pos) }
}

pub(crate) fn index_of(v: &ffi::QList_$1, value: &ffi::$1) -> isize {
    ffi::qlist_index_of_$1(v, value)
}

pub(crate) fn insert(s: &mut ffi::QList_$1, pos: isize, value: &ffi::$1) {
    ffi::qlist_insert_$1(s, pos, value);
}

pub(crate) fn len(s: &ffi::QList_$1) -> isize {
    ffi::qlist_len_$1(s)
}

pub(crate) fn remove(s: &mut ffi::QList_$1, pos: isize) {
    ffi::qlist_remove_$1(s, pos);
}
EOF
    rustfmt "$SCRIPTPATH/qlist_$LOWER.rs"
}

#generate_bridge "QBrush"
#generate_bridge "QTextCharFormat"
#generate_bridge "QTextFormat"
generate_bridge "QTextLength"
