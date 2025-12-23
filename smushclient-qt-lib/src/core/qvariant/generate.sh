#!/usr/bin/env bash

set -e

SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

function generate_bridge_qt() {
    local LOWER
    LOWER=$(echo "$1" | tr '[:upper:]' '[:lower:]')


    tee "$SCRIPTPATH/../../../include/core/qvariant/qvariant_$LOWER.h" <<EOF
//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qvariant/generate.sh

#pragma once
#include <QtCore/QVariant>
#include <smushclient-qt-lib/$LOWER.h>

namespace rust {
namespace cxxqtlib1 {
namespace qvariant {
bool qvariantCanConvert$1(const QVariant& variant);
}
}
}
EOF
    clang-format -i "$SCRIPTPATH/../../../include/core/qvariant/qvariant_$LOWER.h"

    tee "$SCRIPTPATH/qvariant_$LOWER.rs" <<EOF
//! This is an auto-generated file. Do not edit.
//! Edit instead: src/core/qvariant/generate.sh

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;

        include!("smushclient-qt-lib/qvariant_$LOWER.h");
        type $1 = crate::$1;
    }

    #[namespace = "rust::cxxqtlib1::qvariant"]
    unsafe extern "C++" {
        #[rust_name = "qvariant_can_convert_$1"]
        fn qvariantCanConvert$1(variant: &QVariant) -> bool;
        #[rust_name = "qvariant_construct_$1"]
        fn qvariantConstruct(value: &$1) -> QVariant;
        #[rust_name = "qvariant_value_or_default_$1"]
        fn qvariantValueOrDefault(variant: &QVariant) -> $1;
    }
}

pub(crate) fn can_convert(variant: &ffi::QVariant) -> bool {
    ffi::qvariant_can_convert_$1(variant)
}

pub(crate) fn construct(value: &ffi::$1) -> ffi::QVariant {
    ffi::qvariant_construct_$1(value)
}

pub(crate) fn value_or_default(variant: &ffi::QVariant) -> ffi::$1 {
    ffi::qvariant_value_or_default_$1(variant)
}
EOF
    rustfmt +nightly "$SCRIPTPATH/qvariant_$LOWER.rs"
}

generate_bridge_qt "QBrush"
generate_bridge_qt "QTextLength"
generate_bridge_qt "QTextFormat"
