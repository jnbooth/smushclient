#![allow(
    clippy::cast_sign_loss,
    clippy::cast_possible_truncation,
    clippy::cast_precision_loss
)]
use std::error::Error;
use std::fmt;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};

const MILLIS: u32 = 1_000_000;
const NANOS: u32 = 1_000_000_000;
const NANOS_F: f64 = NANOS as f64;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Hmsn(pub u64, pub u64, pub u64, pub u32);

#[allow(clippy::many_single_char_names)]
impl fmt::Display for Hmsn {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let Self(h, m, s, n) = *self;
        if n == 0 {
            write!(f, "{h:02}:{m:02}:{s:02}")
        } else {
            let ms = n / MILLIS;
            write!(f, "{h:02}:{m:02}:{s:02}.{ms:03}")
        }
    }
}

impl Hmsn {
    pub const fn try_from_float_secs(
        hour: u64,
        minute: u64,
        second: f64,
    ) -> Result<Self, TryFromFloatSecsError> {
        if second < 0.0 || second >= 60.0 {
            return Err(TryFromFloatSecsError(second));
        }
        let nanos = (second.fract() * NANOS_F) as u32;
        Ok(Self(hour, minute, second as u64, nanos))
    }

    #[must_use = "this returns a new Hmsn without modifying the original"]
    pub const fn with_hour(&self, hour: u64) -> Self {
        let Hmsn(_, m, s, n) = *self;
        Self(hour, m, s, n)
    }

    #[must_use = "this returns a new Hmsn without modifying the original"]
    pub const fn with_minute(&self, minute: u64) -> Self {
        let Hmsn(h, _, s, n) = *self;
        Self(h, minute, s, n)
    }

    #[must_use = "this returns a new Hmsn without modifying the original"]
    pub const fn with_second(&self, second: u64) -> Self {
        let Hmsn(h, m, _, n) = *self;
        Self(h, m, second, n)
    }

    #[must_use = "this returns a new Hmsn without modifying the original"]
    pub const fn with_nanos(&self, nanos: u32) -> Self {
        let Hmsn(h, m, s, _) = *self;
        Self(h, m, s, nanos)
    }

    #[must_use = "this returns a new Hmsn without modifying the original"]
    pub const fn with_fsecond(&self, second: f64) -> Result<Self, TryFromFloatSecsError> {
        let Hmsn(h, m, _, _) = *self;
        Self::try_from_float_secs(h, m, second)
    }

    pub const fn hour(&self) -> u64 {
        self.0
    }

    pub const fn minute(&self) -> u64 {
        self.1
    }

    pub const fn second(&self) -> u64 {
        self.2
    }

    pub const fn fsecond(&self) -> f64 {
        (self.second() as f64) + (self.nanos() as f64) / NANOS_F
    }

    pub const fn nanos(&self) -> u32 {
        self.3
    }

    pub const fn millis(&self) -> u32 {
        self.3 / 1_000_000
    }

    pub const fn total_seconds(&self) -> u64 {
        let Self(h, m, s, _) = *self;
        h * 3600 + m * 60 + s
    }

    pub const fn normalize_nanos(&mut self) {
        let Self(_, _, s, n) = self;
        if *n < NANOS {
            return;
        }
        *s += (*n / NANOS) as u64;
        *n %= NANOS;
    }
}

impl From<Duration> for Hmsn {
    fn from(value: Duration) -> Self {
        let secs = value.as_secs();
        let nanos = value.subsec_nanos();
        Self(secs / 3600, secs % 3600 / 60, secs % 60, nanos)
    }
}

impl From<Hmsn> for Duration {
    fn from(mut value: Hmsn) -> Self {
        value.normalize_nanos();
        Duration::new(value.total_seconds(), value.nanos())
    }
}

impl From<NaiveTime> for Hmsn {
    fn from(value: NaiveTime) -> Self {
        Self(
            value.hour().into(),
            value.minute().into(),
            value.second().into(),
            value.nanosecond(),
        )
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TimeError(());

impl fmt::Display for TimeError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.write_str("invalid time")
    }
}

impl Error for TimeError {}

impl TryFrom<Hmsn> for NaiveTime {
    type Error = TimeError;

    fn try_from(value: Hmsn) -> Result<Self, Self::Error> {
        let Hmsn(h, m, s, n) = value;
        if h >= 24 || m >= 60 || s >= 60 {
            return Err(TimeError(()));
        }
        NaiveTime::from_hms_nano_opt(h as u32, m as u32, s as u32, n).ok_or(TimeError(()))
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct TryFromFloatSecsError(f64);

impl fmt::Display for TryFromFloatSecsError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "seconds outside of range 0..60: {}", self.0)
    }
}

impl Error for TryFromFloatSecsError {}
