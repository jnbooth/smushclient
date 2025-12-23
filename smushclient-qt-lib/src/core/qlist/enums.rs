#![allow(clippy::wildcard_imports)]
use cxx_qt_lib::{QList, QListElement};

unsafe fn cast<From, To>(list: &QList<From>) -> &QList<To>
where
    From: QListElement,
    To: QListElement,
{
    // SAFETY: Provided by const assertions inside impl_qlist_element.
    unsafe { &*(std::ptr::from_ref(list).cast()) }
}

unsafe fn cast_mut<From, To>(list: &mut QList<From>) -> &mut QList<To>
where
    From: QListElement,
    To: QListElement,
{
    // SAFETY: Provided by const assertions inside impl_qlist_element.
    unsafe { &mut *(std::ptr::from_mut(list).cast()) }
}

macro_rules! impl_qlist_element {
    ($t:ty, $typeId:literal $(,)?) => {
        impl_qlist_element!($t, $typeId, i32);
    };
    ($t:ty, $typeId:literal, $r:ty) => {
        // Assert size equivalency.
        const _: [(); std::mem::size_of::<$t>()] = [(); std::mem::size_of::<$r>()];

        #[allow(clippy::undocumented_unsafe_blocks)]
        impl QListElement for $t {
            type TypeId = cxx::type_id!($typeId);

            fn append(list: &mut QList<Self>, value: Self) {
                <$r as QListElement>::append(unsafe { cast_mut(list) }, value.repr);
            }
            fn append_clone(list: &mut QList<Self>, value: &Self) {
                <$r as QListElement>::append_clone(unsafe { cast_mut(list) }, &value.repr);
            }
            fn clear(list: &mut QList<Self>) {
                <$r as QListElement>::clear(unsafe { cast_mut(list) });
            }
            fn clone(list: &QList<Self>) -> QList<Self> {
                unsafe { std::mem::transmute(<$r as QListElement>::clone(cast(list))) }
            }
            fn contains(list: &QList<Self>, value: &Self) -> bool {
                <$r as QListElement>::contains(unsafe { cast(list) }, &value.repr)
            }
            fn default() -> QList<Self> {
                unsafe { std::mem::transmute(<$r as QListElement>::default()) }
            }
            fn drop(list: &mut QList<Self>) {
                <$r as QListElement>::drop(unsafe { cast_mut(list) })
            }
            unsafe fn get_unchecked(list: &QList<Self>, pos: isize) -> &Self {
                unsafe {
                    &*std::ptr::from_ref(<$r as QListElement>::get_unchecked(cast(list), pos))
                        .cast()
                }
            }
            fn index_of(list: &QList<Self>, value: &Self) -> isize {
                <$r as QListElement>::index_of(unsafe { cast(list) }, &value.repr)
            }
            fn insert(list: &mut QList<Self>, pos: isize, value: Self) {
                <$r as QListElement>::insert(unsafe { cast_mut(list) }, pos, value.repr);
            }
            fn insert_clone(list: &mut QList<Self>, pos: isize, value: &Self) {
                <$r as QListElement>::insert_clone(unsafe { cast_mut(list) }, pos, &value.repr);
            }
            fn len(list: &QList<Self>) -> isize {
                <$r as QListElement>::len(unsafe { cast(list) })
            }
            fn remove(list: &mut QList<Self>, pos: isize) {
                <$r as QListElement>::remove(unsafe { cast_mut(list) }, pos);
            }
            fn reserve(list: &mut QList<Self>, size: isize) {
                <$r as QListElement>::reserve(unsafe { cast_mut(list) }, size);
            }
        }
    };
}

impl_qlist_element!(
    crate::QTextCharFormatUnderlineStyle,
    "QList_QTextCharFormatUnderlineStyle"
);
impl_qlist_element!(
    crate::QTextCharFormatVerticalAlignment,
    "QList_QTextCharFormatVerticalAlignment"
);
impl_qlist_element!(crate::QTextFormatFormatType, "QList_QTextFormatFormatType");
impl_qlist_element!(crate::QTextFormatObjectTypes, "QList_QTextFormatObjectType");
impl_qlist_element!(
    crate::QTextFormatPageBreakFlag,
    "QList_QTextFormatPageBreakFlag"
);
impl_qlist_element!(crate::QTextFormatProperty, "QList_QTextFormatProperty");
impl_qlist_element!(crate::QTextLengthType, "QList_QTextLengthType");
