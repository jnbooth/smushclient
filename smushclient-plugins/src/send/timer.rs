use std::borrow::Cow;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use serde::{Deserialize, Serialize};

use super::occurrence::Occurrence;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::xml::{XmlIterable, bool_serde, is_default};

const NANOS: u64 = 1_000_000_000;
const NANOS_F: f64 = 1_000_000_000.0;

const fn duration_from_hms(hour: u64, minute: u64, second: f64) -> Duration {
    debug_assert!(
        second.is_finite() && (second == 0.0 || second.is_sign_positive()),
        "second must be a finite positive number"
    );
    #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
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
    type Xml<'a> = XmlTimer<'a>;
}

#[derive(Debug, Default, Deserialize, Serialize)]
#[serde(rename = "timer", default = "XmlTimer::template")]
#[rustfmt::skip]
pub struct XmlTimer<'a> {
    #[serde(rename = "@active_closed", with = "bool_serde", skip_serializing_if = "is_default")]
    pub active_closed: bool,
    #[serde(rename = "@at_time", with = "bool_serde", skip_serializing_if = "is_default")]
    pub at_time: bool,
    #[serde(rename = "@enabled", with = "bool_serde")]
    pub enabled: bool,
    #[serde(rename = "@group", borrow, skip_serializing_if = "is_default")]
    pub group: Cow<'a, str>,
    #[serde(rename = "@hour", skip_serializing_if = "is_default")]
    pub hour: u32,
    #[serde(rename = "@minute", skip_serializing_if = "is_default")]
    pub minute: u32,
    #[serde(rename = "@name", borrow, skip_serializing_if = "is_default")]
    pub name: Cow<'a, str>,
    // pub offset_hour: u64,
    // pub offset_minute: u64,
    // pub offset_second: f64,
    #[serde(rename = "@omit_from_log", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_log: bool,
    #[serde(rename = "@omit_from_output", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_output: bool,
    #[serde(rename = "@one_shot", with = "bool_serde", skip_serializing_if = "is_default")]
    pub one_shot: bool,
    #[serde(rename = "@script", borrow, skip_serializing_if = "is_default")]
    pub script: Cow<'a, str>,
    #[serde(rename = "@second", skip_serializing_if = "is_default")]
    pub second: f64,
    #[serde(borrow, skip_serializing_if = "is_default")]
    pub send: Cow<'a, str>,
    #[serde(with = "sendto_serde", rename = "@send_to", skip_serializing_if = "is_default")]
    pub send_to: SendTarget,
    #[serde(rename = "@temporary", with = "bool_serde", skip_serializing_if = "is_default")]
    pub temporary: bool,
    #[serde(rename = "@user", skip_serializing_if = "is_default")]
    pub user: i64,
    #[serde(rename = "@variable", borrow, skip_serializing_if = "is_default")]
    pub variable: Cow<'a, str>,
}
impl XmlTimer<'_> {
    fn template() -> Self {
        Self {
            enabled: true,
            ..Default::default()
        }
    }
}
impl From<XmlTimer<'_>> for Timer {
    fn from(value: XmlTimer) -> Self {
        let occurrence = if value.at_time {
            #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
            let rounded_second = value.second as u32;
            Occurrence::Time(
                NaiveTime::from_hms_opt(value.hour, value.minute, rounded_second)
                    .unwrap_or_default(),
            )
        } else {
            Occurrence::Interval(duration_from_hms(
                value.hour.into(),
                value.minute.into(),
                value.second,
            ))
        };
        Self {
            occurrence,
            active_closed: value.active_closed,
            send: Sender {
                group: value.group.into(),
                label: value.name.into(),
                send_to: value.send_to,
                script: value.script.into(),
                variable: value.variable.into(),
                text: value.send.into(),
                enabled: value.enabled,
                one_shot: value.one_shot,
                temporary: value.temporary,
                omit_from_output: value.omit_from_output,
                omit_from_log: value.omit_from_log,
                id: Sender::get_id(),
                userdata: value.user,
            },
        }
    }
}
impl<'a> From<&'a Timer> for XmlTimer<'a> {
    fn from(value: &'a Timer) -> Self {
        let (at_time, hour, minute, second) = match value.occurrence {
            Occurrence::Interval(every) => {
                #[allow(clippy::cast_possible_truncation)]
                let secs = every.as_secs() as u32;
                let hour = secs / 3600;
                let minute = (secs % 3600) / 3600;
                let second = f64::from(every.subsec_nanos()) / NANOS_F;
                (false, hour, minute, second)
            }
            Occurrence::Time(time) => (true, time.hour(), time.minute(), time.second().into()),
        };
        Self {
            active_closed: value.active_closed,
            at_time,
            enabled: value.enabled,
            group: (&value.group).into(),
            hour,
            minute,
            name: (&value.label).into(),
            omit_from_log: value.omit_from_log,
            omit_from_output: value.omit_from_output,
            one_shot: value.one_shot,
            script: (&value.script).into(),
            second,
            send_to: value.send_to,
            send: (&value.text).into(),
            temporary: value.temporary,
            variable: (&value.variable).into(),
            user: value.userdata,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xml_roundtrip() {
        let timer = Timer::default();
        let xml = timer.to_xml_string().expect("error serializing timer");
        assert_eq!(xml, r#"<timer enabled="y"></timer>"#);
        let mut roundtrip = Timer::from_xml_str(&xml).expect("error deserializing timer");
        roundtrip.id = timer.id;
        assert_eq!(roundtrip, timer);
    }
}
