use rodio::cpal::FromSample;
use rodio::mixer::Mixer;
use rodio::{Source, StreamError};

use super::error::AudioError;
use super::looping::LoopingSink;
use super::stream::AudioStream;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum PlayMode {
    #[default]
    Once,
    Loop,
}

impl From<bool> for PlayMode {
    fn from(value: bool) -> Self {
        if value { Self::Loop } else { Self::Once }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum AudioSinkStatus {
    OutOfRange = -1,
    Done = 0,
    Playing = 1,
    Looping = 2,
}

pub(crate) struct AudioSinks {
    sinks: [LoopingSink; 10],
    stream: AudioStream,
}

impl AudioSinks {
    pub fn try_default() -> Result<Self, StreamError> {
        Ok(Self {
            sinks: Default::default(),
            stream: AudioStream::open_default_stream()?,
        })
    }

    pub fn mixer(&self) -> &Mixer {
        self.stream.mixer()
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

    pub fn play<S>(
        &self,
        i: usize,
        source: S,
        volume: f32,
        mode: PlayMode,
    ) -> Result<(), AudioError>
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

    pub fn configure_sink(&self, i: usize, volume: f32, mode: PlayMode) -> Result<(), AudioError> {
        let sink = self.get(i)?;
        sink.set_volume(volume);
        sink.set_looping(mode == PlayMode::Loop);
        Ok(())
    }

    pub fn status(&self, i: usize) -> AudioSinkStatus {
        let Some(sink) = i.checked_sub(1).and_then(|i| self.sinks.get(i)) else {
            return AudioSinkStatus::OutOfRange;
        };
        if sink.done() {
            AudioSinkStatus::Done
        } else if sink.looping() {
            AudioSinkStatus::Looping
        } else {
            AudioSinkStatus::Playing
        }
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
