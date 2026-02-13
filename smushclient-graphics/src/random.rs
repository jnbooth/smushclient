#[derive(Debug)]
pub(crate) struct NoiseRng {
    inner: fastrand::Rng,
    offset: f64,
    scale: f64,
}

impl NoiseRng {
    pub fn new(threshold: f64) -> Self {
        let threshold = threshold / 100.0;
        Self {
            inner: fastrand::Rng::new(),
            offset: 128.0 * threshold,
            scale: 256.0 * threshold,
        }
    }

    #[inline]
    pub fn next(&mut self) -> f64 {
        self.offset - self.inner.f64() * self.scale
    }
}
