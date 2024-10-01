use std::pin::Pin;

use cxx_qt_lib::QString;

use crate::ffi;

// SAFETY: TreeBuilder instances are only ever provided during UI initialization, which guarantees
// all internally-used pointers are valid.
adapter!(TableBuilderAdapter, ffi::TableBuilder);

impl<'a> TableBuilderAdapter<'a> {
    pub fn set_row_count(&self, rows: i32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.set_row_count(rows) };
    }

    pub fn start_row(&mut self, data: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_row(data) };
    }

    pub fn add_column<T: ColumnInsertable>(&mut self, value: T) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { value.insert_column(self.as_mut()) };
    }
}

pub trait ColumnInsertable {
    unsafe fn insert_column(self, builder: Pin<&mut ffi::TableBuilder>);
}

macro_rules! impl_column_insertable {
    ($t:ty, $i:ident) => {
        impl ColumnInsertable for $t {
            unsafe fn insert_column(self, builder: Pin<&mut ffi::TableBuilder>) {
                builder.$i(self);
            }
        }
    };
}

impl_column_insertable!(&QString, add_column);
impl_column_insertable!(bool, add_column_bool);
