use std::fs::File;
use std::io::BufReader;
use std::path::Path;

use rodio::source::Buffered;
use rodio::{Decoder, Sink, Source, StreamError};

use super::error::AudioError;
use super::stream::AudioStream;

/// Plays a single file.
/// In a settings UI for selecting a sound file, `AudioFilePlayback` provides the ability to play
/// back the file.
pub struct AudioFilePlayback {
    _stream: AudioStream,
    sink: Sink,
    source: Option<Buffered<Decoder<BufReader<File>>>>,
}

impl AudioFilePlayback {
    pub fn try_default() -> Result<Self, StreamError> {
        let stream = AudioStream::open_default_stream()?;
        Ok(Self {
            sink: Sink::connect_new(stream.mixer()),
            _stream: stream,
            source: None,
        })
    }

    pub fn clear(&mut self) {
        self.source = None;
    }

    pub fn play(&self) {
        if let Some(source) = &self.source {
            self.sink.clear();
            self.sink.append(source.clone());
            self.sink.play();
        }
    }

    pub fn set_file<P: AsRef<Path>>(&mut self, path: P) -> Result<(), AudioError> {
        let decoder = Decoder::try_from(File::open(path)?)?;
        self.source = Some(decoder.buffered());
        Ok(())
    }
}
