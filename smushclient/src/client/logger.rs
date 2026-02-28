use std::fs::{File, OpenOptions};
use std::io::{self, BufWriter, Write};

use mud_transformer::Output;

use super::log_file::LogFile;
use crate::world::{Escaped, LogBrackets, LogFormat, LogMode, WorldConfig};

#[derive(Debug)]
pub struct Logger {
    brackets: LogBrackets,
    buf: String,
    file: LogFile,
    format: LogFormat,
    path: Option<String>,
}

impl Logger {
    pub fn new(world: &WorldConfig) -> Self {
        Self {
            brackets: world.brackets(),
            buf: String::new(),
            file: LogFile::default(),
            format: world.log_format,
            path: None,
        }
    }

    pub fn len(&mut self) -> io::Result<Option<u64>> {
        self.file.len()
    }

    pub fn path(&self) -> Option<&str> {
        self.path.as_deref()
    }

    pub fn open(&mut self, path: String, mode: LogMode) -> io::Result<()> {
        self.path = None;
        let file = OpenOptions::from(mode).open(&path)?;
        file.try_lock()?;
        let mut file = BufWriter::new(file);
        self.write_bracket(self.brackets.file.before(), &mut file)?;
        self.file = LogFile::new(file);
        self.path = Some(path);
        Ok(())
    }

    pub fn is_open(&self) -> bool {
        self.file.is_open()
    }

    pub fn apply_world(&mut self, world: &WorldConfig) {
        self.brackets = world.brackets();
        self.format = world.log_format;
    }

    pub fn close(&mut self) -> io::Result<()> {
        let Some(mut file) = self.file.take() else {
            return Ok(());
        };
        self.path = None;
        self.write_bracket(self.brackets.file.after(), &mut file)
    }

    pub fn flush(&mut self) -> io::Result<()> {
        self.file.flush()
    }

    pub fn write_all(&mut self, bytes: &[u8]) -> io::Result<()> {
        self.file.write_all(bytes)
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

    fn write_bracket(&self, bracket: &Escaped, file: &mut BufWriter<File>) -> io::Result<()> {
        if bracket.is_empty() {
            return Ok(());
        }
        let mut buf = String::new();
        let text = bracket.format(&mut buf, None);
        if self.format == LogFormat::Html {
            html_escape::encode_text_to_writer(text, file)?;
        } else {
            file.write_all(text.as_bytes())?;
        }
        file.write_all(b"\n")
    }
}

impl Drop for Logger {
    fn drop(&mut self) {
        let _ = self.close();
    }
}
