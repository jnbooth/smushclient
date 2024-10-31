use std::ops::Range;

use cxx_qt_lib::{QByteArray, QColor, QString};
use enumeration::EnumSet;
use mud_transformer::{TelnetSource, TelnetVerb, TextStyle};

use crate::ffi;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct QColorPair {
    pub foreground: QColor,
    pub background: QColor,
}

// SAFETY: Document instances are owned by their parent WorldTab, ensuring all pointers are valid.
adapter!(DocumentAdapter, ffi::Document);

impl<'a> DocumentAdapter<'a> {
    pub fn append_html(&self, html: &QString) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.append_html(html) };
    }

    pub fn append_line(&mut self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.as_mut().append_line() }
    }

    pub fn append_plaintext(&self, text: &QString, palette: i32) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.append_plaintext(text, palette) };
    }

    pub fn append_text(&self, text: &QString, style: EnumSet<TextStyle>, color: &QColorPair) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.inner
                .append_text(text, style.into(), &color.foreground, &color.background);
        }
    }

    pub fn append_link(
        &self,
        text: &QString,
        style: EnumSet<TextStyle>,
        color: &QColorPair,
        link: &ffi::Link,
    ) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.inner.append_link(
                text,
                style.into(),
                &color.foreground,
                &color.background,
                link,
            );
        }
    }

    pub fn apply_styles(&self, range: Range<usize>, style: EnumSet<TextStyle>, color: &QColorPair) {
        unsafe {
            self.inner.apply_styles(
                i32::try_from(range.start).unwrap_or(i32::MAX),
                i32::try_from(range.end - range.start).unwrap_or(i32::MAX),
                style.into(),
                &color.foreground,
                &color.background,
            );
        }
    }

    pub fn beep(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.beep() };
    }

    pub fn begin(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.begin() };
    }

    pub fn erase_current_line(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.erase_current_line() };
    }

    pub fn erase_last_line(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.erase_last_line() };
    }

    pub fn erase_last_character(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.erase_last_character() };
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

    pub fn handle_telnet_naws(&self) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.handle_telnet_naws() };
    }

    pub fn handle_telnet_negotiation(&self, source: TelnetSource, verb: TelnetVerb, code: u8) {
        // SAFETY: External call to safe method on opaque type.
        unsafe {
            self.inner
                .handle_telnet_negotiation(source.into(), verb.into(), code);
        }
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

    pub fn set_suppress_echo(&self, suppress: bool) {
        // SAFETY: External call to safe method on opaque type.
        unsafe { self.inner.set_suppress_echo(suppress) };
    }
}
