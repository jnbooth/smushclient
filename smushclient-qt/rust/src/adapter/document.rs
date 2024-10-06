use std::ffi::c_char;

use cxx_qt_lib::{QByteArray, QColor, QString, QStringList};
use smushclient_plugins::SendTarget;

use crate::ffi;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct QColorPair {
    pub foreground: QColor,
    pub background: QColor,
}

// SAFETY: Document instances are owned by their parent WorldTab, ensuring all pointers are valid.
adapter!(DocumentAdapter, ffi::Document);

impl<'a> DocumentAdapter<'a> {
    pub fn append_html(&mut self, html: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_html(html) };
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

    pub fn begin(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().begin() };
    }

    pub fn end(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.end() };
    }

    pub fn handle_mxp_change(&self, enabled: bool) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_mxp_change(enabled) };
    }

    pub fn handle_mxp_entity(&self, value: &[u8]) {
        let ptr = value.as_ptr().cast::<c_char>();
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_mxp_entity(ptr, value.len()) };
    }

    pub fn handle_mxp_variable(&self, name: &[u8], value: &[u8]) {
        let name_ptr = name.as_ptr().cast::<c_char>();
        let value_ptr = value.as_ptr().cast::<c_char>();
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.inner
                .handle_mxp_variable(name_ptr, name.len(), value_ptr, value.len());
        }
    }

    pub fn handle_telnet_iac_ga(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_iac_ga() };
    }

    pub fn handle_telnet_request(&self, code: u8, sent: bool) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_request(code, sent) };
    }

    pub fn handle_telnet_subnegotiation(&self, code: u8, data: &QByteArray) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_subnegotiation(code, data) };
    }

    pub fn permit_line(&self, data: &[u8]) -> bool {
        let ptr = data.as_ptr().cast::<c_char>();
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.permit_line(ptr, data.len()) }
    }

    pub fn send(&self, plugin: usize, target: SendTarget, text: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.send(plugin, target.into(), text) };
    }

    pub fn send_script(
        &self,
        plugin: usize,
        script: &QString,
        alias: &QString,
        line: &QString,
        wildcards: &QStringList,
    ) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.inner
                .send_script(plugin, script, alias, line, wildcards);
        }
    }
}
