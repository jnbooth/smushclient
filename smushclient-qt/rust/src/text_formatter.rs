use std::borrow::Cow;
use std::collections::HashMap;

use cxx_qt_lib::QString;
use flagset::FlagSet;
use mud_transformer::mxp::{Link, RgbColor, SendTo};
use mud_transformer::{TextFragment, TextStyle};
use smushclient::{SpanStyle, World};
use smushclient_qt_lib::{QBrush, QFontWeight, QTextCharFormat, QTextFormatProperty};

use crate::convert::Convert;

pub const STYLES_PROPERTY: QTextFormatProperty = QTextFormatProperty::user(0);
pub const PROMPTS_PROPERTY: QTextFormatProperty = QTextFormatProperty::user(1);

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TextFormatter {
    ansi: HashMap<RgbColor, QTextCharFormat>,
    named: HashMap<RgbColor, QTextCharFormat>,
    error_format: QTextCharFormat,
}

impl Default for TextFormatter {
    fn default() -> Self {
        Self::new()
    }
}

fn encode_link(sendto: SendTo, action: &str) -> String {
    match sendto {
        SendTo::Input => format!("i:{action}"),
        SendTo::World => format!("w:{action}"),
        SendTo::Internet => format!("n:{action}"),
    }
}

fn foreground_format(color: RgbColor) -> QTextCharFormat {
    let mut format = QTextCharFormat::default();
    format.set_foreground(&QBrush::from(&color.convert()));
    format
}

impl TextFormatter {
    pub fn new() -> Self {
        let named = RgbColor::iter_named()
            .map(|(_, color)| (color, foreground_format(color)))
            .collect();
        Self {
            ansi: HashMap::with_capacity(17),
            named,
            error_format: QTextCharFormat::default(),
        }
    }

    pub fn apply_world(&mut self, world: &World) {
        self.error_format
            .set_foreground(&QBrush::from(&world.ansi_colours[1].convert()));

        self.ansi.clear();
        for &color in &world.ansi_colours {
            self.ansi.insert(color, foreground_format(color));
        }
        self.ansi
            .insert(RgbColor::BLACK, QTextCharFormat::default());
    }

    pub fn error_format(&self) -> &QTextCharFormat {
        &self.error_format
    }

    fn builder(&self, foreground: RgbColor) -> FormatBuilder<'_> {
        match self.ansi.get(&foreground) {
            Some(format) => FormatBuilder(Cow::Borrowed(format)),
            None => match self.named.get(&foreground) {
                Some(format) => FormatBuilder(Cow::Borrowed(format)),
                None => FormatBuilder(Cow::Owned(foreground_format(foreground))),
            },
        }
    }

    pub fn span_format(&self, style: &SpanStyle) -> Cow<'_, QTextCharFormat> {
        let mut format = self.builder(style.foreground.unwrap_or(RgbColor::WHITE));
        if let Some(background) = style.background {
            format.background(background);
        }
        format.flags(style.flags);
        format.build()
    }

    pub fn text_format(&self, fragment: &TextFragment) -> Cow<'_, QTextCharFormat> {
        let mut format = self.builder(fragment.foreground);
        format.background(fragment.background);
        format.flags(fragment.flags);
        if let Some(link) = &fragment.action {
            format.link(link);
        }
        format.build()
    }
}

struct FormatBuilder<'a>(Cow<'a, QTextCharFormat>);

impl<'a> FormatBuilder<'a> {
    pub fn background(&mut self, background: RgbColor) {
        if background == RgbColor::WHITE {
            return;
        }
        self.0
            .to_mut()
            .set_background(&QBrush::from(&background.convert()));
    }

    pub fn flags(&mut self, flags: FlagSet<TextStyle>) {
        if flags.is_empty() {
            return;
        }
        let format = self.0.to_mut();
        format.set_property(STYLES_PROPERTY, &i32::from(flags.bits()));
        if flags.contains(TextStyle::Bold) {
            format.set_font_weight(QFontWeight::Bold);
        }
        if flags.contains(TextStyle::Italic) {
            format.set_font_italic(true);
        }
        if flags.contains(TextStyle::Strikeout) {
            format.set_font_strike_out(true);
        }
        if flags.contains(TextStyle::Underline) {
            format.set_font_underline(true);
        }
    }

    pub fn link(&mut self, link: &Link) {
        let format = self.0.to_mut();
        format.set_anchor(true);
        format.set_anchor_href(&QString::from(&encode_link(link.sendto, &link.action)));
        if let Some(hint) = &link.hint {
            format.set_tool_tip(&QString::from(hint));
        }
        if !link.prompts.is_empty() {
            format.set_property(PROMPTS_PROPERTY, &QString::from(&link.prompts.join("|")));
        }
    }

    pub fn build(self) -> Cow<'a, QTextCharFormat> {
        self.0
    }
}
