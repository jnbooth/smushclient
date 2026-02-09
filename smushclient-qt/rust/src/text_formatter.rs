use std::borrow::Cow;
use std::collections::HashMap;

use cxx_qt_lib::QColor;
use flagset::FlagSet;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{TextFragment, TextStyle};
use smushclient::{SpanStyle, World};
use smushclient_qt_lib::{QBrush, QTextCharFormat};

use crate::ffi::spans;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TextFormatter {
    base: QTextCharFormat,
    ansi: HashMap<RgbColor, QTextCharFormat>,
    named: HashMap<RgbColor, QTextCharFormat>,
    error_format: QTextCharFormat,
}

impl Default for TextFormatter {
    fn default() -> Self {
        Self::new()
    }
}

fn brush(color: RgbColor) -> QBrush {
    QBrush::from(&QColor::from_rgb(
        color.r.into(),
        color.g.into(),
        color.b.into(),
    ))
}

fn foreground_format(color: RgbColor) -> QTextCharFormat {
    let mut format = QTextCharFormat::default();
    format.set_foreground(&brush(color));
    format
}

impl TextFormatter {
    pub fn new() -> Self {
        let named = RgbColor::iter_named()
            .map(|(_, color)| (color, foreground_format(color)))
            .collect();
        Self {
            base: QTextCharFormat::default(),
            ansi: HashMap::with_capacity(16),
            named,
            error_format: QTextCharFormat::default(),
        }
    }

    pub fn apply_world(&mut self, world: &World) {
        if let Some(color) = world.error_text_colour {
            self.error_format.set_foreground(&brush(color));
        } else {
            self.error_format.clear_foreground();
        }
        if let Some(color) = world.error_background_colour {
            self.error_format.set_background(&brush(color));
        } else {
            self.error_format.clear_background();
        }

        self.ansi.clear();
        for &color in &world.ansi_colours {
            self.ansi.insert(color, foreground_format(color));
        }
    }

    pub fn error_format(&self) -> &QTextCharFormat {
        &self.error_format
    }

    pub fn span_format(&self, style: &SpanStyle) -> Cow<'_, QTextCharFormat> {
        self.get_format(style.foreground, style.background, style.flags)
    }

    pub fn text_format(&self, fragment: &TextFragment) -> Cow<'_, QTextCharFormat> {
        let mut format = self.get_format(fragment.foreground, fragment.background, fragment.flags);
        if let Some(link) = &fragment.action {
            spans::apply_link(format.to_mut(), link);
        }
        format
    }

    fn get_format(
        &self,
        foreground: Option<RgbColor>,
        background: Option<RgbColor>,
        flags: FlagSet<TextStyle>,
    ) -> Cow<'_, QTextCharFormat> {
        let mut format = self.get_foreground_format(foreground);
        if let Some(background) = background {
            format.to_mut().set_background(&brush(background));
        }
        if !flags.is_empty() {
            spans::apply_styles(format.to_mut(), flags);
        }
        format
    }

    fn get_foreground_format(&self, foreground: Option<RgbColor>) -> Cow<'_, QTextCharFormat> {
        let Some(foreground) = foreground else {
            return Cow::Borrowed(&self.base);
        };
        if let Some(format) = self.ansi.get(&foreground) {
            return Cow::Borrowed(format);
        }
        if let Some(format) = self.named.get(&foreground) {
            return Cow::Borrowed(format);
        }
        Cow::Owned(foreground_format(foreground))
    }
}
