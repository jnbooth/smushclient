use std::fs::File;
use std::io::BufReader;
use std::path::Path;

use rodio::source::{Buffered, SamplesConverter};
use rodio::{Decoder, OutputStream, OutputStreamHandle, Source};

use super::error::AudioError;

pub struct AudioPlayback {
    _stream: OutputStream,
    handle: OutputStreamHandle,
    samples: Option<Buffered<SamplesConverter<Decoder<BufReader<File>>, f32>>>,
}

impl AudioPlayback {
    pub fn try_default() -> Result<Self, AudioError> {
        let (stream, handle) = OutputStream::try_default()?;
        Ok(Self {
            _stream: stream,
            handle,
            samples: None,
        })
    }

    pub fn clear(&mut self) {
        self.samples = None;
    }

    pub fn play(&self) -> Result<(), AudioError> {
        if let Some(samples) = &self.samples {
            self.handle.play_raw(samples.clone())?;
        }
        Ok(())
    }

    pub fn set_file<P: AsRef<Path>>(&mut self, path: P) -> Result<(), AudioError> {
        let decoder = Decoder::new(BufReader::new(File::open(path)?))?;
        self.samples = Some(decoder.convert_samples().buffered());
        Ok(())
    }
}
