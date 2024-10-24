use cxx_qt_lib::{QByteArray, QColor, QString};

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
        unsafe { self.inner.append_html(html) };
    }

    pub fn append_line(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_line() }
    }

    pub fn append_plaintext(&mut self, text: &QString, palette: i32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.append_plaintext(text, palette) };
    }

    pub fn append_text(&mut self, text: &QString, style: u16, color: &QColorPair) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.inner
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
            self.inner
                .append_link(text, style, &color.foreground, &color.background, link);
        }
    }

    pub fn begin(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.begin() };
    }

    pub fn echo(&self, command: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.echo(command) };
    }

    pub fn erase_last_line(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.erase_last_line() };
    }

    pub fn end(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.end() };
    }

    pub fn handle_mxp_change(&self, enabled: bool) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_mxp_change(enabled) };
    }

    pub fn handle_mxp_entity(&self, value: &str) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_mxp_entity(value) };
    }

    pub fn handle_mxp_variable(&self, name: &str, value: &str) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_mxp_variable(name, value) };
    }

    pub fn handle_telnet_iac_ga(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_iac_ga() };
    }

    pub fn handle_telnet_request(&self, code: u8, supported: bool) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_request(code, supported) };
    }

    pub fn handle_telnet_subnegotiation(&self, code: u8, data: &QByteArray) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_subnegotiation(code, data) };
    }

    pub fn permit_line(&self, line: &str) -> bool {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.permit_line(line) }
    }

    pub fn play_sound(&self, file_path: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.play_sound(file_path) };
    }

    pub fn send(&self, request: &ffi::SendRequest) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.send(request) };
    }

    pub fn send_script(&self, request: &ffi::SendScriptRequest) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.send_script(request) };
    }
}
