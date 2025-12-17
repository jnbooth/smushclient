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
    _stream: OutputStream,
}

impl AudioSinks {
    pub fn try_default() -> Result<Self, StreamError> {
        let stream = OutputStreamBuilder::open_default_stream()?;
        let mixer = stream.mixer();
        let sinks = [
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
            LoopingSink::connect_new(mixer),
        ];
        Ok(Self {
            sinks,
            _stream: stream,
        })
    }

    fn get(&self, i: usize) -> Result<&LoopingSink, AudioError> {
        if i != 0 {
            return self.sinks.get(i - 1).ok_or(AudioError::SinkOutOfRange);
        }
        match self.sinks.iter().find(|sink| sink.empty()) {
            Some(sink) => Ok(sink),
            None => self.sinks.first().ok_or(AudioError::SinkOutOfRange),
        }
    }

    fn play<S>(&self, i: usize, source: S, volume: f32, mode: PlayMode) -> Result<(), AudioError>
    where
        S: Source + Send + 'static,
        f32: FromSample<S::Item>,
    {
        let sink = self.get(i)?;
        sink.clear();
        sink.set_volume(volume);
        sink.set_looping(mode == PlayMode::Loop);
        sink.append_looping(source);
        sink.play();
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
                sink.clear();
            }
            return Ok(());
        }
        self.get(i)?.clear();
        Ok(())
    }
}
