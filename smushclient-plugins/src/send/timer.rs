use std::borrow::Cow;
use std::time::Duration;

use chrono::NaiveTime;
use serde::de::{Deserializer, Error, Unexpected};
use serde::{Deserialize, Serialize};

use super::occurrence::Occurrence;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::newline::ensure_send_crlf;
use crate::xml::{XmlIterable, bool_serde, is_default};

fn duration_from_hms(hour: u32, minute: u32, second: f64) -> Duration {
    Duration::from_secs(u64::from(hour) * 3600 + u64::from(minute) * 60)
        + Duration::from_secs_f64(second)
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

fn de_hour<'de, D: Deserializer<'de>>(deserializer: D) -> Result<u32, D::Error> {
    let hour = u32::deserialize(deserializer)?;
    if hour >= 24 {
        return Err(D::Error::invalid_value(
            Unexpected::Unsigned(hour.into()),
            &"integer between 0 and 23",
        ));
    }
    Ok(hour)
}

fn de_min<'de, D: Deserializer<'de>>(deserializer: D) -> Result<u32, D::Error> {
    let minute = u32::deserialize(deserializer)?;
    if minute >= 60 {
        return Err(D::Error::invalid_value(
            Unexpected::Unsigned(minute.into()),
            &"integer between 0 and 59",
        ));
    }
    Ok(minute)
}

fn de_sec<'de, D: Deserializer<'de>>(deserializer: D) -> Result<f64, D::Error> {
    let sec = f64::deserialize(deserializer)?;
    if !(0.0..60.0).contains(&sec) {
        return Err(D::Error::invalid_value(
            Unexpected::Float(sec),
            &"number between 0 and 59.9999",
        ));
    }
    Ok(sec)
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
    #[serde(rename = "@hour", deserialize_with = "de_hour", skip_serializing_if = "is_default")]
    pub hour: u32,
    #[serde(rename = "@minute", deserialize_with = "de_min", skip_serializing_if = "is_default")]
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
    #[serde(rename = "@second", deserialize_with = "de_sec", skip_serializing_if = "is_default")]
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
            NaiveTime::from_hms_opt(value.hour, value.minute, rounded_second)
                .unwrap_or_default()
                .into()
        } else {
            duration_from_hms(value.hour, value.minute, value.second).into()
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
                text: ensure_send_crlf(value.send),
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
        Self {
            active_closed: value.active_closed,
            at_time: matches!(value.occurrence, Occurrence::Interval(_)),
            enabled: value.enabled,
            group: (&value.group).into(),
            hour: value.occurrence.hour(),
            minute: value.occurrence.minute(),
            name: (&value.label).into(),
            omit_from_log: value.omit_from_log,
            omit_from_output: value.omit_from_output,
            one_shot: value.one_shot,
            script: (&value.script).into(),
            second: value.occurrence.seconds(),
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
