#![allow(clippy::cast_sign_loss)]
use std::borrow::Cow;
use std::sync::atomic::{AtomicU16, Ordering};
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use serde::{Deserialize, Serialize};

use super::occurrence::Occurrence;
use super::send_to::{sendto_serde, SendTarget};
use super::sender::Sender;
use crate::in_place::InPlace;

const NANOS: u64 = 1_000_000_000;
const NANOS_F: f64 = 1_000_000_000.0;

fn get_id() -> u16 {
    static ID_COUNTER: AtomicU16 = AtomicU16::new(0);
    ID_COUNTER.fetch_add(1, Ordering::Relaxed)
}

fn duration_from_hms(hour: u64, minute: u64, second: f64) -> Duration {
    debug_assert!(
        second.is_finite() && (second == 0.0 || second.is_sign_positive()),
        "second must be a finite positive number"
    );
    #[allow(clippy::cast_possible_truncation)]
    Duration::from_nanos((NANOS_F * second) as u64 + NANOS * 60 * (minute + 60 * hour))
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub struct Timer {
    // Note: this is at the top for Ord-deriving purposes.
    pub send: Sender,
    pub occurrence: Occurrence,
    pub active_closed: bool,
    #[serde(skip, default = "get_id")]
    pub id: u16,
}

impl Default for Timer {
    fn default() -> Self {
        Self {
            send: Sender::default(),
            occurrence: Occurrence::default(),
            active_closed: false,
            id: get_id(),
        }
    }
}

impl_deref!(Timer, Sender, send);
impl_asref!(Timer, Sender);

#[derive(Clone, Debug, Default, PartialEq, PartialOrd, Deserialize, Serialize)]
#[serde(default)]
pub struct TimerXml<'a> {
    #[serde(
        borrow,
        default,
        rename = "@name",
        skip_serializing_if = "str::is_empty"
    )]
    label: Cow<'a, str>,
    #[serde(
        borrow,
        default,
        rename = "@script",
        skip_serializing_if = "str::is_empty"
    )]
    script: Cow<'a, str>,
    #[serde(rename = "@enabled")]
    enabled: bool,
    #[serde(rename = "@at_time")]
    at_time: bool,
    #[serde(
        borrow,
        default,
        rename = "@group",
        skip_serializing_if = "str::is_empty"
    )]
    group: Cow<'a, str>,
    #[serde(
        borrow,
        default,
        rename = "@variable",
        skip_serializing_if = "str::is_empty"
    )]
    variable: Cow<'a, str>,
    #[serde(rename = "@one_shot")]
    one_shot: bool,
    #[serde(rename = "@active_closed")]
    active_closed: bool,
    #[serde(with = "sendto_serde", rename = "@send_to")]
    send_to: SendTarget,
    #[serde(rename = "@omit_from_output")]
    omit_from_output: bool,
    #[serde(rename = "@omit_from_log")]
    omit_from_log: bool,
    #[serde(rename = "@hour")]
    hour: u64,
    #[serde(rename = "@minute")]
    minute: u64,
    #[serde(rename = "@second")]
    second: f64,
    #[serde(rename = "@temporary")]
    temporary: bool,
    #[serde(borrow, default, rename = "send")]
    text: Vec<Cow<'a, str>>,
}
impl From<TimerXml<'_>> for Timer {
    fn from(value: TimerXml) -> Self {
        let occurrence = if value.at_time {
            Occurrence::Time(
                NaiveTime::from_hms_opt(
                    u32::try_from(value.hour).unwrap(),
                    u32::try_from(value.minute).unwrap(),
                    #[allow(clippy::cast_possible_truncation)]
                    u32::try_from(value.second as i64).unwrap(),
                )
                .unwrap(),
            )
        } else {
            Occurrence::Interval(duration_from_hms(value.hour, value.minute, value.second))
        };
        let send = in_place!(
            value,
            Sender {
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
            id: get_id(),
        }
    }
}
impl<'a> From<&'a Timer> for TimerXml<'a> {
    #[allow(clippy::cast_lossless)]
    fn from(value: &'a Timer) -> Self {
        let (at_time, hour, minute, second) = match value.occurrence {
            Occurrence::Interval(every) => {
                let secs = every.as_secs();
                let hour = secs / 3600;
                let minute = (secs % 3600) / 3600;
                let second = every.subsec_nanos() as f64 / NANOS_F;
                (false, hour, minute, second)
            }
            Occurrence::Time(time) => (
                true,
                u64::from(time.hour()),
                u64::from(time.minute()),
                time.second() as f64,
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
