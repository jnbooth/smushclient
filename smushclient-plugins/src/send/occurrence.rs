use std::fmt;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use serde::{Deserialize, Serialize};

use crate::hmsn::{Hmsn, TimeError};

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub enum Occurrence {
    // Note: this is at the top for Ord-deriving purposes.
    Time(NaiveTime),
    Interval(Duration),
}

impl Default for Occurrence {
    fn default() -> Self {
        Self::Interval(Duration::default())
    }
}

impl From<NaiveTime> for Occurrence {
    fn from(value: NaiveTime) -> Self {
        Self::Time(value)
    }
}

impl From<Duration> for Occurrence {
    fn from(value: Duration) -> Self {
        Self::Interval(value)
    }
}

impl From<Occurrence> for Hmsn {
    fn from(value: Occurrence) -> Self {
        match value {
            Occurrence::Time(time) => time.into(),
            Occurrence::Interval(duration) => duration.into(),
        }
    }
}

impl Occurrence {
    pub fn hour(&self) -> u64 {
        match self {
            Self::Time(time) => time.hour().into(),
            Self::Interval(duration) => Hmsn::from(*duration).hour(),
        }
    }

    pub fn minute(&self) -> u64 {
        match self {
            Self::Time(time) => time.minute().into(),
            Self::Interval(duration) => Hmsn::from(*duration).minute(),
        }
    }

    pub fn second(&self) -> u64 {
        match self {
            Self::Time(time) => time.second().into(),
            Self::Interval(duration) => Hmsn::from(*duration).second(),
        }
    }

    pub fn seconds(&self) -> f64 {
        Hmsn::from(*self).fsecond()
    }

    #[must_use = "this returns a new Occurrence without modifying the original"]
    pub fn with_hmsn(&self, hmsn: Hmsn) -> Result<Self, TimeError> {
        match self {
            Self::Time(_) => Ok(Self::Time(hmsn.try_into()?)),
            Self::Interval(_) => Ok(Self::Interval(hmsn.into())),
        }
    }
}

impl fmt::Display for Occurrence {
    #[allow(clippy::many_single_char_names)]
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::Interval(interval) => Hmsn::from(*interval).fmt(f),
            Self::Time(time) => time.format("%-I:%M %p").fmt(f),
        }
    }
}
