use std::fs::{File, OpenOptions};
use std::io::{self, BufWriter, Write};

use chrono::Local;

use crate::world::{Escaped, EscapedBrackets, LogFormat, LogMode, World};

#[derive(Debug)]
pub struct LogFile {
    mode: LogMode,
    format: LogFormat,
    path_template: String,
    path: Option<String>,
    brackets: EscapedBrackets,
    file: Option<BufWriter<File>>,
}

const fn is_retryable(kind: io::ErrorKind) -> bool {
    matches!(kind, io::ErrorKind::NotFound | io::ErrorKind::BrokenPipe)
}

impl LogFile {
    pub fn new(world: &World, brackets: EscapedBrackets) -> Self {
        Self {
            mode: world.log_mode,
            format: world.log_format,
            path_template: world.auto_log_file_name.clone(),
            path: None,
            brackets,
            file: None,
        }
    }

    pub fn path(&self) -> Option<&str> {
        self.path.as_deref()
    }

    pub fn apply_world(&mut self, world: &World, brackets: EscapedBrackets) -> io::Result<()> {
        self.brackets = brackets;
        self.mode = world.log_mode;
        if self.path_template == world.auto_log_file_name {
            return Ok(());
        }
        self.path_template.clone_from(&world.auto_log_file_name);
        if self.file.is_none() {
            return Ok(());
        }
        self.file = None;
        self.open()
    }

    fn can_write(&self) -> bool {
        let Some(file) = &self.file else {
            return false;
        };
        let Ok(metadata) = file.get_ref().metadata() else {
            return false;
        };
        !metadata.permissions().readonly()
    }

    fn write_bracket(&self, bracket: &Escaped, file: &mut BufWriter<File>) -> io::Result<()> {
        if bracket.is_empty() {
            return Ok(());
        }
        let mut buf = String::new();
        let text = bracket.format(&mut buf, None);
        if self.format == LogFormat::Html {
            html_escape::encode_safe_to_writer(text, file)?;
        } else {
            file.write_all(text.as_bytes())?;
        }
        file.write_all(b"\n")
    }

    pub fn open(&mut self) -> io::Result<()> {
        if self.can_write() {
            return Ok(());
        }
        self.path = None;
        let mut path = String::new();
        Local::now()
            .format(&self.path_template)
            .write_to(&mut path)
            .unwrap();
        let file = OpenOptions::from(self.mode).open(&path)?;
        file.try_lock()?;
        let mut file = BufWriter::new(file);
        self.write_bracket(self.brackets.before(), &mut file)?;
        self.file = Some(file);
        self.path = Some(path);
        Ok(())
    }

    pub fn close(&mut self) -> io::Result<()> {
        let Some(mut file) = self.file.take() else {
            return Ok(());
        };
        self.path = None;
        self.write_bracket(self.brackets.after(), &mut file)
    }

    pub fn len(&mut self) -> io::Result<Option<u64>> {
        let Some(file) = &mut self.file else {
            return Ok(None);
        };
        file.flush()?;
        let metadata = file.get_ref().metadata()?;
        Ok(Some(metadata.len()))
    }

    fn do_io<T, F>(&mut self, mut f: F) -> io::Result<T>
    where
        T: Default,
        F: FnMut(&mut BufWriter<File>) -> io::Result<T>,
    {
        let Some(file) = &mut self.file else {
            return Ok(T::default());
        };
        match f(file) {
            Ok(result) => return Ok(result),
            Err(e) if !is_retryable(e.kind()) => {
                self.file = None;
                return Err(e);
            }
            Err(_) => self.file = None,
        }
        self.open()?;
        let Some(file) = &mut self.file else {
            return Ok(T::default());
        };
        match f(file) {
            Ok(result) => Ok(result),
            Err(e) => {
                self.file = None;
                Err(e)
            }
        }
    }
}

impl Write for LogFile {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        self.do_io(|file| file.write(buf))
    }

    fn write_all(&mut self, buf: &[u8]) -> io::Result<()> {
        self.do_io(|file| file.write_all(buf))
    }

    fn write_vectored(&mut self, bufs: &[io::IoSlice<'_>]) -> io::Result<usize> {
        self.do_io(|file| file.write_vectored(bufs))
    }

    fn flush(&mut self) -> io::Result<()> {
        self.do_io(Write::flush)
    }
}

impl io::Seek for LogFile {
    fn seek(&mut self, pos: io::SeekFrom) -> io::Result<u64> {
        self.do_io(|file| file.seek(pos))
    }
}
