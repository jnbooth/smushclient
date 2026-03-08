use cxx_qt::CxxQtType;
use cxx_qt_lib::{QColor, QList};
use smushclient_qt_lib::QPair;

use crate::convert::Convert;
use crate::ffi::{self, StringView};

impl ffi::SmushClient {
    pub fn ansi_note(&self, text: StringView<'_>) -> Vec<ffi::StyledSpan> {
        self.rust().ansi_note(&text.to_string_lossy())
    }

    pub fn color_map(&self) -> QList<QPair<QColor, QColor>> {
        self.rust()
            .client
            .borrow_world()
            .colour_map
            .iter()
            .map(|(k, v)| QPair::from((k.convert(), v.convert())))
            .collect()
    }

    pub fn get_mapped_color(&self, color: &QColor) -> QColor {
        let Some(rgb) = color.convert() else {
            return color.clone();
        };
        match self.rust().client.get_mapped_color(rgb) {
            Some(mapped) => mapped.convert(),
            None => color.clone(),
        }
    }

    pub fn get_term_color(&self, i: u8) -> QColor {
        self.rust().client.xterm_color(i).convert()
    }

    pub fn set_mapped_color(&self, color: &QColor, mapped: &QColor) {
        let Some(color) = color.convert() else {
            return;
        };
        let mapped = mapped.convert().unwrap_or(color);
        self.rust().client.set_mapped_color(color, mapped);
    }

    pub fn set_term_color(&self, i: u8, color: &QColor) {
        self.rust().client.set_xterm_color(i, color.convert());
    }
}
