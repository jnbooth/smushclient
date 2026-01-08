use std::io::{self, Write};

use super::log_file::LogFile;
use crate::world::{LogBrackets, LogFormat, World};

#[derive(Debug)]
pub struct Logger {
    brackets: LogBrackets,
    format: LogFormat,
    file: LogFile,
}

impl Logger {
    pub fn new(world: &World) -> Self {
        let brackets = world.brackets();
        Self {
            file: LogFile::new(world, brackets.file.clone()),
            format: world.log_format,
            brackets: world.brackets(),
        }
    }

    pub fn open(&mut self) -> io::Result<()> {
        self.file.open()
    }

    pub fn apply_world(&mut self, world: &World) -> io::Result<()> {
        self.brackets = world.brackets();
        self.format = world.log_format;
        self.file.apply_world(world, self.brackets.file.clone())
    }

    pub fn close(&mut self) -> io::Result<()> {
        self.file.close()
    }

    pub fn log_raw(&mut self, bytes: &[u8]) -> io::Result<()> {
        if self.format != LogFormat::Raw {
            return Ok(());
        }
        self.file.write_all(bytes)
    }

    pub fn log_input_line<S: AsRef<[u8]>>(&mut self, line: S) -> io::Result<()> {
        if self.format != LogFormat::Text {
            return Ok(());
        }
        self.brackets.input.write(&mut self.file, line.as_ref())
    }

    pub fn log_note<S: AsRef<[u8]>>(&mut self, line: S) -> io::Result<()> {
        if self.format != LogFormat::Text {
            return Ok(());
        }
        self.brackets.notes.write(&mut self.file, line.as_ref())
    }

    pub fn log_output_line<S: AsRef<[u8]>>(&mut self, line: S) -> io::Result<()> {
        if self.format != LogFormat::Text {
            return Ok(());
        }
        self.brackets.output.write(&mut self.file, line.as_ref())
    }
}

impl Drop for Logger {
    fn drop(&mut self) {
        let _ = self.close();
    }
}
