use std::io::{self, Write};

use mud_transformer::Output;

use super::log_file::LogFile;
use crate::world::{LogBrackets, LogFormat, World};

#[derive(Debug)]
pub struct Logger {
    brackets: LogBrackets,
    format: LogFormat,
    file: LogFile,
    buf: String,
}

impl Logger {
    pub fn new(world: &World) -> Self {
        let brackets = world.brackets();
        Self {
            file: LogFile::new(world, brackets.file.clone()),
            format: world.log_format,
            brackets: world.brackets(),
            buf: String::new(),
        }
    }

    pub fn len(&mut self) -> io::Result<Option<u64>> {
        self.file.len()
    }

    pub fn path(&self) -> Option<&str> {
        self.file.path()
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

    pub fn log_input_line(&mut self, line: &str) -> io::Result<()> {
        if self.format == LogFormat::Raw {
            return Ok(());
        }
        self.buf.clear();
        self.brackets
            .input
            .write(&mut self.file, line, &mut self.buf)
    }

    pub fn log_note(&mut self, line: &str) -> io::Result<()> {
        if self.format == LogFormat::Raw {
            return Ok(());
        }
        self.buf.clear();
        self.brackets
            .notes
            .write(&mut self.file, line, &mut self.buf)
    }

    pub fn log_output_line(&mut self, line: &str, fragments: &[Output]) -> io::Result<()> {
        match self.format {
            LogFormat::Raw => Ok(()),
            LogFormat::Html => {
                self.buf.clear();
                self.brackets
                    .output
                    .write_output(&mut self.file, fragments, &mut self.buf)
            }
            LogFormat::Text => {
                self.buf.clear();
                self.brackets
                    .output
                    .write(&mut self.file, line, &mut self.buf)
            }
        }
    }
}

impl Drop for Logger {
    fn drop(&mut self) {
        let _ = self.close();
    }
}
