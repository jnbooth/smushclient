use cxx_qt_lib::{QString, QVariant};
use smushclient_plugins::{Alias, Occurrence, Timer, Trigger};

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

pub trait RowInsertable {
    fn insert_row(&self, builder: &mut TreeBuilderAdapter);
}

impl RowInsertable for Alias {
    fn insert_row(&self, builder: &mut TreeBuilderAdapter) {
        builder.add_column(&QString::from(&self.label));
        builder.add_column(i64::from(self.sequence));
        builder.add_column(&QString::from(&self.pattern));
        builder.add_column(&QString::from(&self.text));
    }
}

impl RowInsertable for Timer {
    fn insert_row(&self, builder: &mut TreeBuilderAdapter) {
        builder.add_column(&QString::from(&self.label));
        builder.add_column(&QString::from(match self.occurrence {
            Occurrence::Interval(_) => "Every",
            Occurrence::Time(_) => "At",
        }));
        builder.add_column(&QString::from(&self.occurrence.to_string()));
        builder.add_column(&QString::from(&self.text));
    }
}

impl RowInsertable for Trigger {
    fn insert_row(&self, builder: &mut TreeBuilderAdapter) {
        builder.add_column(&QString::from(&self.label));
        builder.add_column(i64::from(self.sequence));
        builder.add_column(&QString::from(&self.pattern));
        builder.add_column(&QString::from(&self.text));
    }
}
