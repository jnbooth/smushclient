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
} // namespace qvariant
} // namespace cxxqtlib1
} // namespace rust
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

#[cfg(test)]
mod tests {
    #[test]
    fn can_convert() {
        assert!(!super::can_convert(&cxx_qt_lib::QVariant::default()));
    }
}
EOF
    rustfmt +nightly "$SCRIPTPATH/qvariant_$LOWER.rs"
}

function generate_bridge_qt_in_place() {
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
} // namespace qvariant
} // namespace cxxqtlib1
} // namespace rust
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
    }

    #[namespace = "rust::cxxqtio1::qvariant"]
    unsafe extern "C++" {
        include!("cxx-qt-io/qvariant.h");

        #[rust_name = "qvariant_init_value_or_default_$1"]
        unsafe fn qvariantInitValueOrDefault(variant: &QVariant, uninit: *mut $1);
    }
}

pub(crate) fn can_convert(variant: &ffi::QVariant) -> bool {
    ffi::qvariant_can_convert_$1(variant)
}

pub(crate) fn construct(value: &ffi::$1) -> ffi::QVariant {
    ffi::qvariant_construct_$1(value)
}

pub(crate) fn value_or_default(variant: &ffi::QVariant) -> ffi::$1 {
    let mut uninit = std::mem::MaybeUninit::uninit();
    // SAFETY: uninit.as_mut_ptr() is valid.
    unsafe { ffi::qvariant_init_value_or_default_$1(variant, uninit.as_mut_ptr()) };
    // SAFETY: ffi::qvariant_init_value_or_default_$1 initializes the passed pointer in-place.
    unsafe { uninit.assume_init() }
}

#[cfg(test)]
mod tests {
    #[test]
    fn can_convert() {
        assert!(!super::can_convert(&cxx_qt_lib::QVariant::default()));
    }
}
EOF
    rustfmt +nightly "$SCRIPTPATH/qvariant_$LOWER.rs"
}

generate_bridge_qt "QTextLength"

generate_bridge_qt_in_place "QBrush"
generate_bridge_qt_in_place "QTextFormat"
