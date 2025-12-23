#![allow(non_camel_case_types)]
#![allow(clippy::needless_lifetimes)]
#![allow(clippy::trivially_copy_pass_by_ref)]
#![allow(clippy::undocumented_unsafe_blocks)]
use cxx::type_id;
use cxx_qt_lib::{QMap, QMapPair};

macro_rules! impl_qmap_pair {
    ( $keyTypeName:ty, $valueTypeName:ty, $module:ident, $pairTypeName:ident, $typeId:literal $(,)? ) => {
        mod $module;
        pub use $module::$pairTypeName;

        impl QMapPair for $module::$pairTypeName {
            type Key = $keyTypeName;
            type Value = $valueTypeName;
            type TypeId = type_id!($typeId);

            fn clear(map: &mut QMap<Self>) {
                $module::clear(map);
            }

            fn clone(map: &QMap<Self>) -> QMap<Self> {
                $module::clone(map)
            }

            fn contains(map: &QMap<Self>, key: &$keyTypeName) -> bool {
                $module::contains(map, key)
            }

            fn default() -> QMap<Self> {
                $module::default()
            }

            fn drop(map: &mut QMap<Self>) {
                $module::drop(map);
            }

            fn get_or_default(map: &QMap<Self>, key: &$keyTypeName) -> $valueTypeName {
                $module::get_or_default(map, key)
            }

            unsafe fn get_unchecked_key(map: &QMap<Self>, pos: isize) -> &$keyTypeName {
                unsafe { $module::get_unchecked_key(map, pos) }
            }

            unsafe fn get_unchecked_value(map: &QMap<Self>, pos: isize) -> &$valueTypeName {
                unsafe { $module::get_unchecked_value(map, pos) }
            }

            fn insert(map: &mut QMap<Self>, key: $keyTypeName, value: $valueTypeName) {
                $module::insert(map, &key, &value);
            }

            fn insert_clone(map: &mut QMap<Self>, key: &$keyTypeName, value: &$valueTypeName) {
                $module::insert(map, key, value);
            }

            fn len(map: &QMap<Self>) -> isize {
                $module::len(map)
            }

            fn remove(map: &mut QMap<Self>, key: &$keyTypeName) -> bool {
                $module::remove(map, key)
            }
        }
    };
}

impl_qmap_pair!(
    i32,
    cxx_qt_lib::QVariant,
    qmap_i32_qvariant,
    QMapPair_i32_QVariant,
    "QMap_i32_QVariant",
);
