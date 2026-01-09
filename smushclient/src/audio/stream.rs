use std::ops::{Deref, DerefMut};

use rodio::{OutputStream, OutputStreamBuilder, StreamError};

#[repr(transparent)]
pub struct AudioStream(OutputStream);

impl AudioStream {
    /// Try to open a new output stream for the default output device with its default configuration.
    /// Failing that attempt to open output stream with alternative configuration and/or non default
    /// output devices. Returns stream for first of the tried configurations that succeeds.
    /// If all attempts fail return the initial error.
    pub fn open_default_stream() -> Result<Self, StreamError> {
        let mut stream = OutputStreamBuilder::open_default_stream()?;
        stream.log_on_drop(false);
        Ok(Self(stream))
    }
}

impl Deref for AudioStream {
    type Target = OutputStream;

    #[inline]
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for AudioStream {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

// SAFETY: `OutputStream` contains an `Arc<Mutex<StreamInner>>`, but `StreamInner` contains `*mut`s,
// causing Rust to mark it as !Send and !Sync. Nevertheless, its usage is safe.
unsafe impl Send for AudioStream {}

// SAFETY: `OutputStream` contains an `Arc<Mutex<StreamInner>>`, but `StreamInner` contains `*mut`s,
// causing Rust to mark it as !Send and !Sync. Nevertheless, its usage is safe.
unsafe impl Sync for AudioStream {}
