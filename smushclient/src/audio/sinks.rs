use std::fs::File;
use std::io::{BufReader, Cursor, Read, Seek};
use std::path::Path;

use super::error::AudioError;
use rodio::cpal::FromSample;
use rodio::{Decoder, OutputStream, Sample, Sink, Source};

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum PlayMode {
    Once,
    Loop,
}

impl From<bool> for PlayMode {
    fn from(value: bool) -> Self {
        if value {
            Self::Loop
        } else {
            Self::Once
        }
    }
}

pub struct AudioSinks {
    sinks: [Sink; 10],
    _stream: OutputStream,
}

trait PlaySound {
    fn play_source<S>(&self, source: S, volume: f32)
    where
        S: Source + Send + 'static,
        f32: FromSample<S::Item>,
        S::Item: Sample + Send;

    fn play_sound<R: Read + Seek + Send + Sync + 'static>(
        &self,
        reader: R,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        match mode {
            PlayMode::Once => self.play_source(Decoder::new(reader)?, volume),
            PlayMode::Loop => self.play_source(Decoder::new_looped(reader)?, volume),
        };
        Ok(())
    }
}

impl PlaySound for Sink {
    fn play_source<S>(&self, source: S, volume: f32)
    where
        S: Source + Send + 'static,
        f32: FromSample<S::Item>,
        S::Item: Sample + Send,
    {
        self.stop();
        self.set_volume(volume);
        self.append(source);
        self.play();
    }
}

impl AudioSinks {
    pub fn try_default() -> Result<Self, AudioError> {
        let (stream, handle) = OutputStream::try_default()?;
        let sinks = [
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
            Sink::try_new(&handle)?,
        ];
        Ok(Self {
            sinks,
            _stream: stream,
        })
    }

    fn get(&self, i: usize) -> Result<&Sink, AudioError> {
        if i != 0 {
            return self.sinks.get(i - 1).ok_or(AudioError::SinkOutOfRange);
        }
        match self.sinks.iter().find(|sink| sink.empty()) {
            Some(sink) => Ok(sink),
            None => self.sinks.first().ok_or(AudioError::SinkOutOfRange),
        }
    }

    pub fn play<R: Read + Seek + Send + Sync + 'static>(
        &self,
        i: usize,
        reader: R,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        self.get(i)?.play_sound(reader, volume, mode)
    }

    #[inline(always)] // avoid move operation on buffer
    pub fn play_buffer(
        &self,
        i: usize,
        buffer: Vec<u8>,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        self.get(i)?.play_sound(Cursor::new(buffer), volume, mode)
    }

    pub fn play_file<P: AsRef<Path>>(
        &self,
        i: usize,
        path: P,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError> {
        self.get(i)?
            .play_sound(BufReader::new(File::open(path)?), volume, mode)
    }

    pub fn set_volume(&self, i: usize, volume: f32) -> Result<(), AudioError> {
        self.get(i)?.set_volume(volume);
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
