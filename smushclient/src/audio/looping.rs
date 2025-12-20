use std::sync::atomic::{AtomicBool, AtomicU32, AtomicUsize, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use rodio::cpal::FromSample;
use rodio::mixer::Mixer;
use rodio::source::{Done, SeekError};
use rodio::{ChannelCount, SampleRate, Source};

struct Controls {
    current: AtomicU32,
    volume: Mutex<f32>,
}

pub struct LoopingSink {
    controls: Arc<Controls>,
    looping: Arc<AtomicBool>,
    sound_count: Arc<AtomicUsize>,
}

impl Default for LoopingSink {
    fn default() -> Self {
        Self::new()
    }
}

impl LoopingSink {
    #[inline]
    pub fn new() -> Self {
        Self {
            controls: Arc::new(Controls {
                current: AtomicU32::new(0),
                volume: Mutex::new(1.0),
            }),
            looping: Arc::default(),
            sound_count: Arc::default(),
        }
    }

    #[inline]
    pub fn done(&self) -> bool {
        self.sound_count.load(Ordering::Relaxed) == 0
    }

    #[inline]
    pub fn stop(&self) {
        self.controls.current.fetch_add(1, Ordering::Relaxed);
    }

    #[inline]
    pub fn set_looping(&self, looping: bool) {
        self.looping.store(looping, Ordering::Relaxed);
    }

    #[inline]
    pub fn set_volume(&self, value: f32) {
        *self.controls.volume.lock().expect("mutex error") = value;
    }

    #[inline]
    pub fn play<S>(&self, source: S, mixer: &Mixer)
    where
        S: Source + Send + 'static,
        f32: FromSample<S::Item>,
    {
        let controls = self.controls.clone();
        let source_index = controls.current.fetch_add(1, Ordering::Relaxed) + 1;
        let source = Looping::new(source, self.looping.clone())
            .amplify(1.0)
            .skippable()
            .periodic_access(Duration::from_millis(100), move |src| {
                if controls.current.load(Ordering::Relaxed) != source_index {
                    src.skip();
                    return;
                }
                let volume = *controls.volume.lock().expect("mutex error");
                src.inner_mut().set_factor(volume);
            });
        self.sound_count.fetch_add(1, Ordering::Relaxed);
        let source = Done::new(source, self.sound_count.clone());
        mixer.add(source);
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
