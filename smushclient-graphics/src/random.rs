pub(crate) struct DissolveRng {
    inner: fastrand::Rng,
    opacity: f64,
}

impl DissolveRng {
    pub fn new(opacity: f64) -> Self {
        Self {
            inner: fastrand::Rng::new(),
            opacity,
        }
    }

    #[inline]
    pub fn erase(&mut self) -> bool {
        self.inner.f64() >= self.opacity
    }
}

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
