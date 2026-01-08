use std::borrow::Cow;
use std::fmt::Write as _;
use std::io::{self, Write};

use chrono::{DateTime, Local};
use mud_transformer::mxp::RgbColor;
use mud_transformer::{Output, OutputFragment};

use super::World;
use crate::world::LogFormat;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Escaped<S = String> {
    message: S,
    has_chrono: bool,
}

impl<'a> Escaped<Cow<'a, str>> {
    pub fn borrow(message: &'a str, world: &World) -> Self {
        if message.len() <= 1 {
            return Escaped {
                message: message.into(),
                has_chrono: false,
            };
        }
        let mut has_world = false;
        let mut has_player = false;
        let mut has_chrono = false;
        let mut in_escape = false;
        for &byte in message.as_bytes() {
            if in_escape {
                match byte {
                    b'N' => has_world = true,
                    b'P' => has_player = true,
                    _ => has_chrono = true,
                }
                in_escape = false;
            } else if byte == b'%' {
                in_escape = true;
            }
        }
        let mut message = Cow::Borrowed(message);
        if has_world {
            message = message.replace("%N", &world.name).into();
        }
        if has_player {
            message = message.replace("%P", &world.player).into();
        }
        Escaped {
            message,
            has_chrono,
        }
    }
}

impl Escaped<String> {
    pub fn for_log(message: &str, world: &World) -> Self {
        let Escaped {
            message,
            has_chrono,
        } = Escaped::borrow(message, world);
        let message = if world.log_format == LogFormat::Html
            && let Cow::Owned(escaped) = html_escape::encode_text(&message)
        {
            escaped
        } else {
            message.into_owned()
        };
        Self {
            message,
            has_chrono,
        }
    }
}

impl<S: AsRef<str>> Escaped<S> {
    pub fn is_empty(&self) -> bool {
        self.message.as_ref().is_empty()
    }

    pub fn format<'a>(&'a self, buf: &'a mut String, now: Option<DateTime<Local>>) -> &'a str {
        let message = self.message.as_ref();
        if !self.has_chrono {
            return message;
        }
        let len = buf.len();
        now.unwrap_or_else(Local::now)
            .format(message)
            .write_to(buf)
            .unwrap();
        &buf[len..]
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
enum EscapeMode {
    #[default]
    Text,
    Html,
    HtmlColor,
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct EscapedBrackets {
    before: Escaped<String>,
    after: Escaped<String>,
    escape_mode: EscapeMode,
}

impl EscapedBrackets {
    pub const fn before(&self) -> &Escaped {
        &self.before
    }

    pub const fn after(&self) -> &Escaped {
        &self.after
    }

    pub fn set_color(&mut self, color: RgbColor) {
        write!(self.before.message, "<span style=\"color:#{color:X}\">").unwrap();
        self.after.message = format!("</span>{}", self.after.message);
    }

    fn get_now(&self) -> Option<DateTime<Local>> {
        if self.before.has_chrono || self.after.has_chrono {
            Some(Local::now())
        } else {
            None
        }
    }

    pub fn write<W: Write>(&self, mut writer: W, line: &str, buf: &mut String) -> io::Result<()> {
        let now = self.get_now();
        writer.write_all(self.before.format(buf, now).as_bytes())?;
        if self.escape_mode == EscapeMode::Text {
            writer.write_all(line.as_bytes())?;
        } else {
            html_escape::encode_safe_to_writer(line, &mut writer)?;
        }
        writer.write_all(self.after.format(buf, now).as_bytes())
    }

    pub fn write_output<W: Write>(
        &self,
        mut writer: W,
        output: &[Output],
        buf: &mut String,
    ) -> io::Result<()> {
        let now = self.get_now();
        writer.write_all(self.before.format(buf, now).as_bytes())?;
        for output_item in output {
            match &output_item.fragment {
                OutputFragment::Hr => writer.write_all(b"<hr>")?,
                OutputFragment::LineBreak => writer.write_all(b"<br>")?,
                OutputFragment::Text(fragment) if self.escape_mode != EscapeMode::HtmlColor => {
                    html_escape::encode_text_to_writer(&fragment.text, &mut writer)?;
                }
                OutputFragment::Text(fragment) => write!(writer, "{}", fragment.html())?,
                _ => (),
            }
        }
        writer.write_all(self.after.format(buf, now).as_bytes())
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct LogBrackets {
    pub file: EscapedBrackets,
    pub output: EscapedBrackets,
    pub input: EscapedBrackets,
    pub notes: EscapedBrackets,
}

impl From<&World> for LogBrackets {
    fn from(world: &World) -> Self {
        let escape_mode = if world.log_format != LogFormat::Html {
            EscapeMode::Text
        } else if world.log_in_colour {
            EscapeMode::HtmlColor
        } else {
            EscapeMode::Html
        };
        let mut this = Self {
            file: EscapedBrackets {
                before: Escaped::for_log(&world.log_file_preamble, world),
                after: Escaped::for_log(&world.log_file_postamble, world),
                escape_mode,
            },
            output: EscapedBrackets {
                before: Escaped::for_log(&world.log_line_preamble_output, world),
                after: Escaped::for_log(&world.log_line_postamble_output, world),
                escape_mode,
            },
            input: EscapedBrackets {
                before: Escaped::for_log(&world.log_line_preamble_input, world),
                after: Escaped::for_log(&world.log_line_postamble_input, world),
                escape_mode,
            },
            notes: EscapedBrackets {
                before: Escaped::for_log(&world.log_line_preamble_notes, world),
                after: Escaped::for_log(&world.log_line_postamble_notes, world),
                escape_mode,
            },
        };
        this.output.after.message.push('\n');
        this.input.after.message.push('\n');
        this.notes.after.message.push('\n');
        if escape_mode != EscapeMode::HtmlColor {
            return this;
        }
        if let Some(echo_colour) = world.echo_colour {
            this.input.set_color(echo_colour);
        }
        if let Some(note_text_colour) = world.note_text_colour {
            this.notes.set_color(note_text_colour);
        }
        this
    }
}
