use cxx_qt_lib::{QColor, QString};

use crate::ffi;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct QColorPair {
    pub foreground: QColor,
    pub background: QColor,
}

// SAFETY: Document instances are only ever provided by WorldTab instances, which guarantees all
// internally-used pointers are valid.
adapter!(DocumentAdapter, ffi::Document);

impl<'a> DocumentAdapter<'a> {
    pub fn scroll_to_bottom(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().scroll_to_bottom() }
    }

    pub fn append_line(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_line() }
    }

    pub fn append_plaintext(&mut self, text: &QString, palette: i32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_plaintext(text, palette) };
    }

    pub fn append_text(&mut self, text: &QString, style: u16, color: &QColorPair) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.as_mut()
                .append_text(text, style, &color.foreground, &color.background);
        }
    }

    pub fn append_link(
        &mut self,
        text: &QString,
        style: u16,
        color: &QColorPair,
        link: &ffi::Link,
    ) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.as_mut()
                .append_link(text, style, &color.foreground, &color.background, link);
        }
    }

    pub fn display_status_message(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().display_status_message(text) };
    }

    pub fn set_input(&mut self, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.as_mut().set_input(text) };
    }
}
