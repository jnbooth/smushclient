use cxx_qt_lib::QString;

use crate::ffi;
use std::pin::Pin;

// SAFETY: TreeBuilder instances are only ever provided during UI initialization, which guarantees
// all internally-used pointers are valid.
adapter!(TreeBuilderAdapter, ffi::TreeBuilder);

impl<'a> TreeBuilderAdapter<'a> {
    pub fn start_group(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_group(text) };
    }

    pub fn start_item(&mut self, value: usize) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_item(value) };
    }

    pub fn add_column<T: ColumnInsertable>(&mut self, value: T) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { value.insert_column(self.as_mut()) };
    }
}

pub trait ColumnInsertable {
    unsafe fn insert_column(self, builder: Pin<&mut ffi::TreeBuilder>);
}

macro_rules! impl_column_insertable {
    ($t:ty, $i:ident) => {
        impl ColumnInsertable for $t {
            unsafe fn insert_column(self, builder: Pin<&mut ffi::TreeBuilder>) {
                builder.$i(self);
            }
        }
    };
}

impl_column_insertable!(&QString, add_column);
impl_column_insertable!(i64, add_column_signed);
impl_column_insertable!(u64, add_column_unsigned);
impl_column_insertable!(f64, add_column_floating);
