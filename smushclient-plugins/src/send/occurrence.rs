use std::fmt::{self, Display, Formatter};
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

impl Display for Occurrence {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        match self {
            Self::Interval(every) => {
                let secs = every.as_secs();
                let millis = every.subsec_millis();
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
            Self::Time(at) => write!(f, "{}", at.format("%-I:%M %p")),
        }
    }
}
