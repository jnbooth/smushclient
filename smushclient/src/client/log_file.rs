use std::fs::File;
use std::io::{self, BufWriter, Write};

#[derive(Debug, Default)]
pub struct LogFile {
    file: Option<BufWriter<File>>,
}

impl LogFile {
    pub fn new(file: BufWriter<File>) -> Self {
        Self { file: Some(file) }
    }

    pub fn is_open(&self) -> bool {
        self.file.is_some()
    }

    pub fn take(&mut self) -> Option<BufWriter<File>> {
        self.file.take()
    }

    pub fn len(&mut self) -> io::Result<Option<u64>> {
        let Some(file) = &mut self.file else {
            return Ok(None);
        };
        file.flush()?;
        let metadata = file.get_ref().metadata()?;
        Ok(Some(metadata.len()))
    }

    #[inline]
    fn do_io<T, F>(&mut self, f: F) -> io::Result<T>
    where
        T: Default,
        F: FnOnce(&mut BufWriter<File>) -> io::Result<T>,
    {
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
