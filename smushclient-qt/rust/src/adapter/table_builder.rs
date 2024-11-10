use std::pin::Pin;

use cxx_qt_lib::{QString, QVariant};
use smushclient_plugins::Plugin;

use crate::ffi;

// SAFETY: TreeBuilder instances are only ever provided during UI initialization, which guarantees
// all internally-used pointers are valid.
adapter!(TableBuilderAdapter, ffi::TableBuilder);

impl<'a> TableBuilderAdapter<'a> {
    pub fn set_row_count(&self, rows: i32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.set_row_count(rows) };
    }

    pub fn start_row<D: Into<QVariant>>(&mut self, data: D) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().start_row(&data.into()) };
    }

    pub fn add_column<T: ColumnInsertable>(&mut self, value: T) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { value.insert_column(self.as_mut()) };
    }

    pub fn add_row<T: RowInsertable, D: Into<QVariant>>(&mut self, data: D, item: &T) {
        self.start_row(data);
        item.insert_row(self);
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

pub trait RowInsertable {
    fn insert_row(&self, builder: &mut TableBuilderAdapter);
}

impl RowInsertable for Plugin {
    fn insert_row(&self, builder: &mut TableBuilderAdapter) {
        let metadata = &self.metadata;
        builder.add_column(&QString::from(&metadata.name));
        builder.add_column(&QString::from(&metadata.purpose));
        builder.add_column(&QString::from(&metadata.author));
        builder.add_column(&QString::from(&*metadata.path.to_string_lossy()));
        builder.add_column(!self.disabled);
        builder.add_column(&QString::from(&metadata.version));
    }
}
