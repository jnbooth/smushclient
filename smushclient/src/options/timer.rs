use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use smushclient_plugins::{Occurrence, Timer};

use super::decode::DecodeOption;
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;
use crate::get_info::InfoVisitor;

fn to_hms(secs: u64) -> (u64, u64, u64) {
    (secs / 3600, secs % 3600 / 60, secs % 60)
}

fn from_hms(h: u64, m: u64, s: u64) -> u64 {
    h * 3600 + m * 60 + s
}

fn set_at_time(occurrence: &mut Occurrence, at_time: bool) {
    match *occurrence {
        Occurrence::Interval(duration) if at_time => {
            let secs = duration.as_secs();
            let nanos = duration.subsec_nanos();
            if let Ok(secs) = secs.try_into()
                && let Some(time) = NaiveTime::from_num_seconds_from_midnight_opt(secs, nanos)
            {
                *occurrence = Occurrence::Time(time);
            }
        }
        Occurrence::Time(time) if !at_time => {
            *occurrence = Occurrence::Interval(Duration::new(
                time.num_seconds_from_midnight().into(),
                time.nanosecond() % 1_000_000_000,
            ));
        }
        _ => (),
    }
}

impl Optionable for Timer {
    fn get_option<V: InfoVisitor>(&self, name: &LuaStr) -> V::Output {
        match name {
            b"active_closed" => V::visit(self.active_closed),
            b"at_time" => V::visit(matches!(self.occurrence, Occurrence::Time(_))),
            b"hour" => V::visit(self.occurrence.hour()),
            b"minute" => V::visit(self.occurrence.minute()),
            b"second" => match self.occurrence {
                Occurrence::Interval(_) => V::visit(self.occurrence.seconds()),
                Occurrence::Time(time) => V::visit(time.second()),
            },
            b"offset_hour" | b"offset_minute" | b"offset_second" => V::visit(0),
            _ => self.send.get_option::<V>(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"active_closed" => self.active_closed = value.decode()?,
            b"at_time" => set_at_time(&mut self.occurrence, value.decode()?),
            b"hour" => {
                self.occurrence = match self.occurrence {
                    Occurrence::Interval(duration) => {
                        let (_, m, s) = to_hms(duration.as_secs());
                        let secs = from_hms(value.decode_in_range(..24)?, m, s);
                        Occurrence::Interval(Duration::new(secs, duration.subsec_nanos()))
                    }
                    Occurrence::Time(time) => Occurrence::Time(
                        time.with_hour(value.decode()?)
                            .ok_or(OptionError::OptionOutOfRange)?,
                    ),
                }
            }
            b"minute" => {
                self.occurrence = match self.occurrence {
                    Occurrence::Interval(duration) => {
                        let (h, _, s) = to_hms(duration.as_secs());
                        let secs = from_hms(h, value.decode_in_range(..60)?, s);
                        Occurrence::Interval(Duration::new(secs, duration.subsec_nanos()))
                    }
                    Occurrence::Time(time) => Occurrence::Time(
                        time.with_minute(value.decode()?)
                            .ok_or(OptionError::OptionOutOfRange)?,
                    ),
                }
            }
            b"second" => {
                self.occurrence = match self.occurrence {
                    Occurrence::Interval(duration) => {
                        let s = value.decode_in_range(..60.0)?;
                        let offset = Duration::try_from_secs_f64(s)?;
                        let (h, m, _) = to_hms(duration.as_secs());
                        let secs = from_hms(h, m, 0);
                        Occurrence::Interval(Duration::new(secs, 0) + offset)
                    }
                    Occurrence::Time(time) => Occurrence::Time(
                        time.with_second(value.decode()?)
                            .ok_or(OptionError::OptionOutOfRange)?,
                    ),
                }
            }
            _ => self.send.set_option(name, value)?,
        }
        Ok(())
    }
}
