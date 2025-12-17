use std::ops::{Deref, DerefMut};
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use std::time::Duration;

use rodio::cpal::FromSample;
use rodio::mixer::Mixer;
use rodio::source::SeekError;
use rodio::{ChannelCount, SampleRate, Sink, Source};

pub struct LoopingSink {
    inner: Sink,
    looping: Arc<AtomicBool>,
}

impl LoopingSink {
    #[inline]
    pub fn connect_new(mixer: &Mixer) -> Self {
        Self {
            inner: Sink::connect_new(mixer),
            looping: Arc::default(),
        }
    }

    #[inline]
    pub fn set_looping(&self, looping: bool) {
        self.looping.store(looping, Ordering::Relaxed);
    }

    #[inline]
    pub fn append_looping<S>(&self, source: S)
    where
        S: Source + Send + 'static,
        f32: FromSample<S::Item>,
    {
        self.inner
            .append(Looping::new(source, self.looping.clone()));
    }
}

impl Deref for LoopingSink {
    type Target = Sink;

    #[inline]
    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

impl DerefMut for LoopingSink {
    #[inline]
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

#[derive(Clone, Debug)]
struct Looping<I> {
    input: I,
    looping: Arc<AtomicBool>,
}

impl<I> Looping<I> {
    #[inline]
    pub fn new(input: I, looping: Arc<AtomicBool>) -> Self {
        Self { input, looping }
    }
}

impl<I> Source for Looping<I>
where
    I: Source,
{
    #[inline]
    fn current_span_len(&self) -> Option<usize> {
        self.input.current_span_len()
    }

    #[inline]
    fn channels(&self) -> ChannelCount {
        self.input.channels()
    }

    #[inline]
    fn sample_rate(&self) -> SampleRate {
        self.input.sample_rate()
    }

    #[inline]
    fn total_duration(&self) -> Option<Duration> {
        if self.looping.load(Ordering::Relaxed) {
            None
        } else {
            self.input.total_duration()
        }
    }

    #[inline]
    fn try_seek(&mut self, pos: Duration) -> Result<(), SeekError> {
        self.input.try_seek(pos)
    }
}

impl<I> Iterator for Looping<I>
where
    I: Source,
{
    type Item = I::Item;

    #[inline]
    fn next(&mut self) -> Option<I::Item> {
        if let Some(sample) = self.input.next() {
            return Some(sample);
        }
        if !self.looping.load(Ordering::Relaxed) {
            return None;
        }
        self.input.try_seek(Duration::ZERO).ok()?;
        self.input.next()
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        self.input.size_hint()
    }
}
