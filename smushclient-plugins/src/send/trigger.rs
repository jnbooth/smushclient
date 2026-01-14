use std::borrow::Cow;

use flagset::{FlagSet, flags};
use mxp::RgbColor;
use serde::{Deserialize, Serialize};

use super::reaction::Reaction;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::in_place::{InPlace, in_place};
use crate::xml::{XmlIterable, bool_serde};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
pub struct Trigger {
    // Note: this is at the top for Ord-deriving purposes.
    pub reaction: Reaction,
    pub change_foreground: bool,
    pub foreground_color: RgbColor,
    pub change_background: bool,
    pub background_color: RgbColor,
    pub make_bold: bool,
    pub make_italic: bool,
    pub make_underline: bool,
    pub sound: String,
    pub sound_if_inactive: bool,
    pub lowercase_wildcard: bool,
    pub multi_line: bool,
    pub lines_to_match: u8,
    pub clipboard_arg: u8,
}

impl Default for Trigger {
    fn default() -> Self {
        Self {
            reaction: Reaction::default(),
            change_foreground: false,
            foreground_color: RgbColor::WHITE,
            change_background: false,
            background_color: RgbColor::BLACK,
            make_bold: false,
            make_italic: false,
            make_underline: false,
            sound: String::new(),
            sound_if_inactive: false,
            lowercase_wildcard: false,
            multi_line: false,
            lines_to_match: 0,
            clipboard_arg: 0,
        }
    }
}

impl_deref!(Trigger, Reaction, reaction);
impl_asref!(Trigger, Reaction);
impl_asref!(Trigger, Sender);

impl XmlIterable for Trigger {
    const TAG: &'static str = "trigger";
    type Xml<'a> = TriggerXml<'a>;
}

flags! {
    #[derive(PartialOrd, Ord, Hash)]
    enum Change: u8 {
        Both,
        Fg,
        Bg,
    }
}

fn get_color(name: &str) -> Option<RgbColor> {
    if name.is_empty() {
        return None;
    }
    RgbColor::named(name)
}

#[allow(clippy::trivially_copy_pass_by_ref)]
const fn is_zero(n: &u8) -> bool {
    *n == 0
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
#[serde(default = "TriggerXml::template")]
#[rustfmt::skip]
pub struct TriggerXml<'a> {
    /// See [`Change`].
    #[serde(rename = "@color_change_type", skip_serializing_if = "is_zero")]
    colour_change_type: u8,
    #[serde(rename = "@enabled", with = "bool_serde")]
    enabled: bool,
    #[serde(rename = "@expand_variables", with = "bool_serde")]
    expand_variables: bool,
    #[serde(rename = "@group", borrow, default, skip_serializing_if = "str::is_empty")]
    group: Cow<'a, str>,
    #[serde(rename = "@ignore_case", with = "bool_serde")]
    ignore_case: bool,
    #[serde(rename = "@lines_to_match")]
    lines_to_match: u8,
    #[serde(rename = "@keep_evaluating", with = "bool_serde")]
    keep_evaluating: bool,
    #[serde(rename = "@make_bold", with = "bool_serde")]
    make_bold: bool,
    #[serde(rename = "@make_italic", with = "bool_serde")]
    make_italic: bool,
    #[serde(rename = "@make_underline", with = "bool_serde")]
    make_underline: bool,
    #[serde(rename = "@match", borrow, default, skip_serializing_if = "str::is_empty")]
    pattern: Cow<'a, str>,
    #[serde(rename = "@multi_line", with = "bool_serde")]
    multi_line: bool,
    #[serde(rename = "@name", borrow, default, skip_serializing_if = "str::is_empty")]
    label: Cow<'a, str>,
    #[serde(rename = "@one_shot", with = "bool_serde")]
    one_shot: bool,
    #[serde(rename = "@omit_from_log", with = "bool_serde")]
    omit_from_log: bool,
    #[serde(rename = "@omit_from_output", with = "bool_serde")]
    omit_from_output: bool,
    #[serde(rename = "@regexp", with = "bool_serde")]
    is_regex: bool,
    #[serde(rename = "@repeats", with = "bool_serde")]
    repeats: bool,
    #[serde(rename = "@script", borrow, default, skip_serializing_if = "str::is_empty")]
    script: Cow<'a, str>,
    #[serde(with = "sendto_serde", rename = "@send_to")]
    send_to: SendTarget,
    #[serde(rename = "@sequence")]
    sequence: i16,
    #[serde(rename = "@sound", borrow, default, skip_serializing_if = "str::is_empty")]
    sound: Cow<'a, str>,
    #[serde(rename = "@sound_if_inactive", with = "bool_serde")]
    sound_if_inactive: bool,
    #[serde(rename = "@lowercase_wildcard", with = "bool_serde")]
    lowercase_wildcard: bool,
    #[serde(rename = "@temporary", with = "bool_serde")]
    temporary: bool,
    #[serde(rename = "@variable", borrow, default, skip_serializing_if = "str::is_empty")]
    variable: Cow<'a, str>,
    #[serde(rename = "@other_text_colour", skip_serializing_if = "String::is_empty")]
    other_text_colour: String,
    #[serde(rename = "@other_back_colour", skip_serializing_if = "String::is_empty")]
    other_back_colour: String,
    #[serde(rename = "@clipboard_arg", default, skip_serializing_if = "is_zero")]
    clipboard_arg: u8,
    #[serde(borrow, default, rename = "send")]
    text: Vec<Cow<'a, str>>,
}
impl TriggerXml<'_> {
    fn template() -> Self {
        Self {
            enabled: true,
            ..Default::default()
        }
    }
}
impl TryFrom<TriggerXml<'_>> for Trigger {
    type Error = crate::regex::RegexError;

    fn try_from(value: TriggerXml) -> Result<Self, Self::Error> {
        let regex = Reaction::make_regex(&value.pattern, value.is_regex, value.ignore_case)?;
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
        let reaction = in_place!(
            value,
            Reaction {
                send,
                regex: regex.into(),
                ..sequence,
                ..pattern,
                ..is_regex,
                ..ignore_case,
                ..keep_evaluating,
                ..expand_variables,
                ..repeats,
            }
        );
        let color_changes = FlagSet::new_truncated(value.colour_change_type);
        Ok(in_place!(
            value,
            Self {
                reaction,
                change_foreground: color_changes.contains(Change::Fg),
                foreground_color: get_color(&value.other_text_colour).unwrap_or(RgbColor::WHITE),
                change_background: color_changes.contains(Change::Bg),
                background_color: get_color(&value.other_back_colour).unwrap_or(RgbColor::BLACK),
                ..sound,
                ..make_bold,
                ..make_italic,
                ..make_underline,
                ..sound_if_inactive,
                ..lowercase_wildcard,
                ..multi_line,
                ..lines_to_match,
                ..clipboard_arg,
            }
        ))
    }
}
impl<'a> From<&'a Trigger> for TriggerXml<'a> {
    fn from(value: &'a Trigger) -> Self {
        let mut color_changes = FlagSet::empty();
        let other_text_colour = if value.change_foreground {
            color_changes |= Change::Fg;
            value.foreground_color.to_string()
        } else {
            String::new()
        };
        let other_back_colour = if value.change_background {
            color_changes |= Change::Bg;
            value.background_color.to_string()
        } else {
            String::new()
        };
        let colour_change_type = color_changes.bits();
        in_place!(
            value,
            Self {
                colour_change_type,
                other_text_colour,
                other_back_colour,
                ..sound,
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
                ..sequence,
                ..pattern,
                ..is_regex,
                ..ignore_case,
                ..keep_evaluating,
                ..expand_variables,
                ..make_bold,
                ..make_italic,
                ..make_underline,
                ..sound_if_inactive,
                ..repeats,
                ..lowercase_wildcard,
                ..multi_line,
                ..lines_to_match,
                ..clipboard_arg,
            }
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xml_roundtrip() {
        let trigger = Trigger::default();
        let to_xml = quick_xml::se::to_string(&TriggerXml::from(&trigger))
            .expect("error serializing trigger");
        let from_xml: TriggerXml =
            quick_xml::de::from_str(&to_xml).expect("error deserializing trigger");
        let roundtrip = Trigger::try_from(from_xml).expect("error converting trigger");
        assert_eq!(roundtrip, trigger);
    }
}
