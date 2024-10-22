use crate::world::{LogFormat, LogMode, World};
use crate::Handler;
use std::fs::File;
use std::io::{self, Write};

#[derive(Debug)]
pub enum Logger {
    Closed,
    Failed(io::Error),
    Open(File, Vec<u8>),
}

impl Default for Logger {
    fn default() -> Self {
        Self::Closed
    }
}

impl Logger {
    pub fn open(world: &World) -> io::Result<Self> {
        let Some(log_file_path) = &world.auto_log_file_name else {
            return Ok(Self::Closed);
        };
        let file = match world.log_mode {
            LogMode::Append => File::open(log_file_path),
            LogMode::Overwrite => File::create(log_file_path),
        }?;
        Ok(Self::Open(file, Vec::new()))
    }

    pub fn log_raw(&mut self, bytes: &[u8], world: &World) {
        if world.log_format != LogFormat::Raw {
            return;
        }
        let Self::Open(file, ..) = self else {
            return;
        };
        if let Err(e) = file.write_all(bytes) {
            *self = Self::Failed(e);
        }
    }

    pub fn log_input_line(&mut self, line: &[u8], world: &World) {
        if world.log_format != LogFormat::Text {
            return;
        }
        self.log_line(line, &world.log_preamble_input, &world.log_postamble_input);
    }

    pub fn log_note(&mut self, line: &[u8], world: &World) {
        if world.log_format != LogFormat::Text {
            return;
        }
        self.log_line(line, &world.log_preamble_notes, &world.log_postamble_notes);
    }

    pub fn log_output_line(&mut self, line: &[u8], world: &World) {
        if world.log_format != LogFormat::Text {
            return;
        }
        self.log_line(
            line,
            &world.log_preamble_output,
            &world.log_postamble_output,
        );
    }

    fn log_line(&mut self, line: &[u8], preamble: &str, postamble: &str) {
        let Self::Open(file, buf) = self else {
            return;
        };
        buf.clear();
        buf.extend_from_slice(preamble.as_bytes());
        buf.extend_from_slice(line);
        buf.extend_from_slice(postamble.as_bytes());
        buf.push(b'\n');
        if let Err(e) = file.write_all(buf) {
            *self = Self::Failed(e);
        }
    }

    pub fn log_error<H: Handler>(&mut self, handler: &mut H) {
        let Self::Failed(e) = self else {
            return;
        };
        handler.display_error(&format!("Log error: {e}"));
        *self = Self::Closed;
    }
}
