use std::fmt;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use serde::{Deserialize, Serialize};

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

impl Occurrence {
    pub fn hour(&self) -> u32 {
        match self {
            Self::Time(time) => time.hour(),
            Self::Interval(duration) => {
                u32::try_from(duration.as_secs() / 3600).unwrap_or(u32::MAX)
            }
        }
    }

    pub fn minute(&self) -> u32 {
        match self {
            Self::Time(time) => time.minute(),
            Self::Interval(duration) => u32::try_from(duration.as_secs() / 60).unwrap_or(u32::MAX),
        }
    }

    pub fn second(&self) -> u32 {
        match self {
            Self::Time(time) => time.second(),
            Self::Interval(duration) => u32::try_from(duration.as_secs()).unwrap_or(u32::MAX),
        }
    }
}

impl fmt::Display for Occurrence {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let interval = match self {
            Self::Interval(interval) => interval,
            Self::Time(time) => return time.format("%-I:%M %p").fmt(f),
        };

        if f.width().is_some() || f.precision().is_some() {
            #[allow(clippy::recursive_format_impl)]
            return f.pad(&self.to_string());
        }

        let secs = interval.as_secs();
        let millis = interval.subsec_millis();
        if millis == 0 {
            write!(
                f,
                "{:02}:{:02}:{:02}",
                secs / 3600,
                (secs % 3600) / 60,
                secs % 60
            )
        } else {
            write!(
                f,
                "{:02}:{:02}:{:02}.{:03}",
                secs / 3600,
                (secs % 3600) / 60,
                secs % 60,
                millis
            )
        }
    }
}
