use std::fs::File;
use std::io::BufReader;
use std::path::Path;

use rodio::source::Buffered;
use rodio::{Decoder, OutputStream, OutputStreamBuilder, Sink, Source, StreamError};

use super::error::AudioError;

/// Plays a single file.
/// In a settings UI for selecting a sound file, `AudioFilePlayback` provides the ability to play
/// back the file.
pub struct AudioFilePlayback {
    _stream: OutputStream,
    sink: Sink,
    source: Option<Buffered<Decoder<BufReader<File>>>>,
}

impl AudioFilePlayback {
    pub fn try_default() -> Result<Self, StreamError> {
        let mut stream = OutputStreamBuilder::open_default_stream()?;
        stream.log_on_drop(false);
        let mixer = stream.mixer();
        let sink = Sink::connect_new(mixer);
        Ok(Self {
            _stream: stream,
            sink,
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
