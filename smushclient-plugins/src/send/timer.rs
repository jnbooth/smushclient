#![allow(clippy::cast_possible_truncation)]
#![allow(clippy::cast_sign_loss)]
use std::borrow::Cow;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use serde::{Deserialize, Serialize};

use super::occurrence::Occurrence;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::in_place::{InPlace, in_place};
use crate::xml::{XmlIterable, bool_serde};

const NANOS: u64 = 1_000_000_000;
const NANOS_F: f64 = 1_000_000_000.0;

const fn duration_from_hms(hour: u64, minute: u64, second: f64) -> Duration {
    debug_assert!(
        second.is_finite() && (second == 0.0 || second.is_sign_positive()),
        "second must be a finite positive number"
    );
    Duration::from_nanos((NANOS_F * second) as u64 + NANOS * 60 * (minute + 60 * hour))
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
pub struct Timer {
    // Note: this is at the top for Ord-deriving purposes.
    pub send: Sender,
    pub occurrence: Occurrence,
    pub active_closed: bool,
}

impl_deref!(Timer, Sender, send);
impl_asref!(Timer, Sender);

impl XmlIterable for Timer {
    const TAG: &'static str = "timer";
    type Xml<'a> = TimerXml<'a>;
}

#[derive(Clone, Debug, Default, PartialEq, PartialOrd, Deserialize, Serialize)]
#[serde(default)]
#[rustfmt::skip]
pub struct TimerXml<'a> {
    #[serde(rename = "@name", borrow, default, skip_serializing_if = "str::is_empty")]
    label: Cow<'a, str>,
    #[serde(rename = "@script", borrow, default, skip_serializing_if = "str::is_empty")]
    script: Cow<'a, str>,
    #[serde(rename = "@enabled", with = "bool_serde")]
    enabled: bool,
    #[serde(rename = "@at_time", with = "bool_serde")]
    at_time: bool,
    #[serde(rename = "@group", borrow, default, skip_serializing_if = "str::is_empty")]
    group: Cow<'a, str>,
    #[serde(rename = "@variable", borrow, default, skip_serializing_if = "str::is_empty")]
    variable: Cow<'a, str>,
    #[serde(rename = "@one_shot", with = "bool_serde")]
    one_shot: bool,
    #[serde(rename = "@active_closed", with = "bool_serde")]
    active_closed: bool,
    #[serde(with = "sendto_serde", rename = "@send_to")]
    send_to: SendTarget,
    #[serde(rename = "@omit_from_output", with = "bool_serde")]
    omit_from_output: bool,
    #[serde(rename = "@omit_from_log", with = "bool_serde")]
    omit_from_log: bool,
    #[serde(rename = "@hour")]
    hour: u64,
    #[serde(rename = "@minute")]
    minute: u64,
    #[serde(rename = "@second")]
    second: f64,
    #[serde(rename = "@temporary", with = "bool_serde")]
    temporary: bool,
    #[serde(borrow, default, rename = "send")]
    text: Vec<Cow<'a, str>>,
}
impl From<TimerXml<'_>> for Timer {
    fn from(value: TimerXml) -> Self {
        let occurrence = if value.at_time {
            Occurrence::Time(
                NaiveTime::from_hms_opt(
                    value.hour as u32,
                    value.minute as u32,
                    value.second as u32,
                )
                .unwrap_or_default(),
            )
        } else {
            Occurrence::Interval(duration_from_hms(value.hour, value.minute, value.second))
        };
        let send = in_place!(
            value,
            Sender {
                    id: Sender::get_id(),
                    userdata: 0,
                    ..label,
                    ..text,
                    ..send_to,
                    ..script,
                    ..group,
                    ..variable,
                    ..enabled,
                    ..one_shot,
                    ..temporary,
                    ..omit_from_output,
                    ..omit_from_log,
            }
        );
        Self {
            occurrence,
            send,
            active_closed: value.active_closed,
        }
    }
}
impl<'a> From<&'a Timer> for TimerXml<'a> {
    fn from(value: &'a Timer) -> Self {
        let (at_time, hour, minute, second) = match value.occurrence {
            Occurrence::Interval(every) => {
                let secs = every.as_secs();
                let hour = secs / 3600;
                let minute = (secs % 3600) / 3600;
                let second = f64::from(every.subsec_nanos()) / NANOS_F;
                (false, hour, minute, second)
            }
            Occurrence::Time(time) => (
                true,
                time.hour().into(),
                time.minute().into(),
                time.second().into(),
            ),
        };
        in_place!(
            value,
            Self {
                at_time,
                hour,
                minute,
                second,
                ..label,
                ..text,
                ..send_to,
                ..script,
                ..group,
                ..variable,
                ..enabled,
                ..one_shot,
                ..temporary,
                ..omit_from_output,
                ..omit_from_log,
                ..active_closed,
            }
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xml_roundtrip() {
        let timer = Timer::default();
        let to_xml =
            quick_xml::se::to_string(&TimerXml::from(&timer)).expect("error serializing trigger");
        let from_xml: TimerXml =
            quick_xml::de::from_str(&to_xml).expect("error deserializing trigger");
        let mut roundtrip = Timer::from(from_xml);
        roundtrip.id = timer.id;
        assert_eq!(roundtrip, timer);
    }
}
