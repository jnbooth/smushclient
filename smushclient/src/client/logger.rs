use crate::handler::Handler;
use crate::world::{Escaped, EscapedBrackets, LogBrackets, LogFormat, LogMode, World};
use std::fs::{File, OpenOptions};
use std::io::{self, BufWriter, Write};
use std::mem;

#[derive(Debug)]
pub enum LogFile {
    Closed,
    Failed(io::Error),
    Open(BufWriter<File>),
}

impl Default for LogFile {
    fn default() -> Self {
        Self::Closed
    }
}

impl LogFile {
    pub fn open(world: &World, preamble: &Escaped) -> io::Result<Self> {
        let Some(log_file_path) = &world.auto_log_file_name else {
            return Ok(Self::Closed);
        };
        let mut options = OpenOptions::new();
        match world.log_mode {
            LogMode::Append => options.append(true),
            LogMode::Overwrite => options.create(true).truncate(true),
        };
        let file = options.create(true).open(log_file_path)?;
        let mut file = BufWriter::new(file);
        if !preamble.is_empty() {
            writeln!(file, "{preamble}")?;
        }
        Ok(Self::Open(file))
    }

    pub fn close(&mut self, postamble: &Escaped) {
        if postamble.is_empty() {
            *self = Self::Closed;
            return;
        }
        let Self::Open(file) = self else {
            return;
        };
        if let Err(e) = writeln!(file, "{postamble}") {
            *self = Self::Failed(e);
        } else {
            *self = Self::Closed;
        }
    }

    pub const fn is_open(&self) -> bool {
        matches!(self, Self::Open(_))
    }

    pub fn log_raw(&mut self, bytes: &[u8]) {
        let Self::Open(file) = self else {
            return;
        };
        let Err(e) = file.write_all(bytes) else {
            return;
        };
        *self = Self::Failed(e);
    }

    pub fn log_line(&mut self, line: &[u8], brackets: &EscapedBrackets) {
        let Self::Open(file) = self else {
            return;
        };
        let Err(e) = brackets.write(file, line) else {
            return;
        };
        *self = Self::Failed(e);
    }

    pub fn take_error(&mut self) -> Option<io::Error> {
        if !matches!(self, Self::Failed(_)) {
            return None;
        }
        let mut buf = Self::Closed;
        mem::swap(self, &mut buf);
        match buf {
            Self::Failed(e) => Some(e),
            _ => None,
        }
    }
}

#[derive(Debug, Default)]
pub struct Logger {
    brackets: LogBrackets,
    file: LogFile,
    format: LogFormat,
}

impl Logger {
    pub fn open(world: &World) -> io::Result<Self> {
        let brackets = world.brackets();
        Ok(Self {
            file: LogFile::open(world, brackets.file.before())?,
            brackets,
            format: world.log_format,
        })
    }

    pub fn apply_world(&mut self, world: &World) {
        self.brackets = world.brackets();
        self.format = world.log_format;
    }

    pub fn close(&mut self) {
        self.file.close(self.brackets.file.after());
    }

    pub const fn is_open(&self) -> bool {
        self.file.is_open()
    }

    pub fn log_raw(&mut self, bytes: &[u8]) {
        if self.format != LogFormat::Raw {
            return;
        }
        self.file.log_raw(bytes);
    }

    pub fn log_input_line(&mut self, line: &[u8]) {
        if self.format != LogFormat::Text {
            return;
        }
        self.file.log_line(line, &self.brackets.input);
    }

    pub fn log_note(&mut self, line: &[u8]) {
        if self.format != LogFormat::Text {
            return;
        }
        self.file.log_line(line, &self.brackets.notes);
    }

    pub fn log_output_line(&mut self, line: &[u8]) {
        if self.format != LogFormat::Text {
            return;
        }
        self.file.log_line(line, &self.brackets.output);
    }

    pub fn log_error<H: Handler>(&mut self, handler: &mut H) {
        if let Some(e) = self.file.take_error() {
            handler.display_error(&format!("Log error: {e}"));
        }
    }
}

impl Drop for Logger {
    fn drop(&mut self) {
        self.close();
    }
}
