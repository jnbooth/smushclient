use std::fs::File;
use std::io::Cursor;
use std::path::Path;

use rodio::cpal::FromSample;
use rodio::{Decoder, OutputStream, OutputStreamBuilder, Source, StreamError};

use super::error::AudioError;
use super::looping::LoopingSink;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum PlayMode {
    Once,
    Loop,
}

impl From<bool> for PlayMode {
    fn from(value: bool) -> Self {
        if value { Self::Loop } else { Self::Once }
    }
}

pub struct AudioSinks {
    sinks: [LoopingSink; 10],
    stream: OutputStream,
}

impl AudioSinks {
    pub fn try_default() -> Result<Self, StreamError> {
        let stream = OutputStreamBuilder::open_default_stream()?;
        let sinks = [
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
            LoopingSink::new(),
        ];
        Ok(Self { sinks, stream })
    }

    fn get(&self, i: usize) -> Result<&LoopingSink, AudioError> {
        if i != 0 {
            return self.sinks.get(i - 1).ok_or(AudioError::SinkOutOfRange);
        }
        match self.sinks.iter().find(|sink| sink.done()) {
            Some(sink) => Ok(sink),
            None => Ok(&self.sinks[0]),
        }
    }

    fn play<S>(&self, i: usize, source: S, volume: f32, mode: PlayMode) -> Result<(), AudioError>
    where
        S: Source + Send + 'static,
        f32: FromSample<S::Item>,
    {
        let sink = self.get(i)?;
        sink.stop();
        sink.set_volume(volume);
        sink.set_looping(mode == PlayMode::Loop);
        sink.play(source, self.stream.mixer());
        Ok(())
    }

    #[inline(always)] // avoid move operation on buffer
    pub fn play_buffer(
        &self,
        i: usize,
        buffer: Vec<u8>,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        let decoder = Decoder::try_from(Cursor::new(buffer))?;
        self.play(i, decoder, volume, mode)
    }

    pub fn play_file<P: AsRef<Path>>(
        &self,
        i: usize,
        path: P,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        let file = File::open(path)?;
        let decoder = Decoder::try_from(file)?;
        self.play(i, decoder, volume, mode)
    }

    pub fn configure_sink(&self, i: usize, volume: f32, mode: PlayMode) -> Result<(), AudioError> {
        let sink = self.get(i)?;
        sink.set_volume(volume);
        sink.set_looping(mode == PlayMode::Loop);
        Ok(())
    }

    pub fn stop(&self, i: usize) -> Result<(), AudioError> {
        if i == 0 {
            for sink in &self.sinks {
                sink.stop();
            }
            return Ok(());
        }
        self.get(i)?.stop();
        Ok(())
    }
}
