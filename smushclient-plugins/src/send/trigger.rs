use std::borrow::Cow;

use mxp::RgbColor;
use serde::{Deserialize, Serialize};

use super::reaction::Reaction;
use super::send_to::{sendto_serde, SendTarget};
use super::sender::Sender;
use crate::in_place::InPlace;

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
pub struct Trigger {
    // Note: this is at the top for Ord-deriving purposes.
    pub reaction: Reaction,
    pub change_foreground: bool,
    pub foreground: String,
    #[serde(skip)]
    pub foreground_color: Option<RgbColor>,
    pub change_background: bool,
    pub background: String,
    #[serde(skip)]
    pub background_color: Option<RgbColor>,
    pub make_bold: bool,
    pub make_italic: bool,
    pub make_underline: bool,
    pub sound: String,
    pub sound_if_inactive: bool,
    pub lowercase_wildcard: bool,
    pub multi_line: bool,
    pub lines_to_match: u8,
}

impl Trigger {
    pub fn new() -> Self {
        Self::default()
    }
}

impl_deref!(Trigger, Reaction, reaction);
impl_asref!(Trigger, Reaction);
impl_asref!(Trigger, Sender);

#[repr(u8)]
enum Change {
    Both,
    Fg,
    Bg,
}

impl Change {
    const fn member(self, set: Option<u8>) -> bool {
        match set {
            None => false,
            Some(0) => true,
            Some(x) => x == self as u8,
        }
    }

    const fn insert_into(self, set: Option<u8>) -> Option<u8> {
        match set {
            None => Some(self as u8),
            Some(i) if i == self as u8 => Some(i),
            _ => Some(Self::Both as u8),
        }
    }
}

fn get_color(name: &str) -> Option<RgbColor> {
    if name.is_empty() {
        return None;
    }
    RgbColor::named(name)
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
#[serde(default = "TriggerXml::template")]
pub struct TriggerXml<'a> {
    /// See [`Change`].
    #[serde(rename = "@color_change_type", skip_serializing_if = "Option::is_none")]
    colour_change_type: Option<u8>,
    #[serde(rename = "@enabled")]
    enabled: bool,
    #[serde(rename = "@expand_variables")]
    expand_variables: bool,
    #[serde(
        borrow,
        default,
        rename = "@group",
        skip_serializing_if = "str::is_empty"
    )]
    group: Cow<'a, str>,
    #[serde(rename = "@ignore_case")]
    ignore_case: bool,
    #[serde(rename = "@lines_to_match")]
    lines_to_match: u8,
    #[serde(rename = "@keep_evaluating")]
    keep_evaluating: bool,
    #[serde(rename = "@make_bold")]
    make_bold: bool,
    #[serde(rename = "@make_italic")]
    make_italic: bool,
    #[serde(rename = "@make_underline")]
    make_underline: bool,
    #[serde(
        borrow,
        default,
        rename = "@match",
        skip_serializing_if = "str::is_empty"
    )]
    pattern: Cow<'a, str>,
    #[serde(rename = "@multi_line")]
    multi_line: bool,
    #[serde(
        borrow,
        default,
        rename = "@name",
        skip_serializing_if = "str::is_empty"
    )]
    label: Cow<'a, str>,
    #[serde(rename = "@one_shot")]
    one_shot: bool,
    #[serde(rename = "@omit_from_log")]
    omit_from_log: bool,
    #[serde(rename = "@omit_from_output")]
    omit_from_output: bool,
    #[serde(rename = "@regexp")]
    is_regex: bool,
    #[serde(rename = "@repeats")]
    repeats: bool,
    #[serde(
        borrow,
        default,
        rename = "@script",
        skip_serializing_if = "str::is_empty"
    )]
    script: Cow<'a, str>,
    #[serde(with = "sendto_serde", rename = "@send_to")]
    send_to: SendTarget,
    #[serde(rename = "@sequence")]
    sequence: i16,
    #[serde(
        borrow,
        default,
        rename = "@sound",
        skip_serializing_if = "str::is_empty"
    )]
    sound: Cow<'a, str>,
    #[serde(rename = "@sound_if_inactive")]
    sound_if_inactive: bool,
    #[serde(rename = "@lowercase_wildcard")]
    lowercase_wildcard: bool,
    #[serde(rename = "@temporary")]
    temporary: bool,
    #[serde(
        borrow,
        default,
        rename = "@variable",
        skip_serializing_if = "str::is_empty"
    )]
    variable: Cow<'a, str>,
    #[serde(
        rename = "@other_text_colour",
        skip_serializing_if = "String::is_empty"
    )]
    other_text_colour: String,
    #[serde(
        rename = "@other_back_colour",
        skip_serializing_if = "String::is_empty"
    )]
    other_back_colour: String,
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
        let regex = Reaction::make_regex(&value.pattern, value.is_regex)?;
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
        let reaction = in_place!(
            value,
            Reaction {
                send,
                regex,
                ..sequence,
                ..pattern,
                ..is_regex,
                ..ignore_case,
                ..keep_evaluating,
                ..expand_variables,
                ..repeats,
            }
        );
        Ok(in_place!(
            value,
            Self {
                reaction,
                change_foreground: Change::Fg.member(value.colour_change_type),
                foreground_color: get_color(&value.other_text_colour),
                foreground: value.other_text_colour,
                change_background:Change::Bg.member(value.colour_change_type),
                background_color: get_color(&value.other_back_colour),
                background: value.other_back_colour,
                ..sound,
                ..make_bold,
                ..make_italic,
                ..make_underline,
                ..sound_if_inactive,
                ..lowercase_wildcard,
                ..multi_line,
                ..lines_to_match,
            }
        ))
    }
}
impl<'a> From<&'a Trigger> for TriggerXml<'a> {
    fn from(value: &'a Trigger) -> Self {
        let mut colour_change_type = None;
        let other_text_colour = if value.change_foreground {
            colour_change_type = Change::Fg.insert_into(colour_change_type);
            value.foreground.clone()
        } else {
            String::new()
        };
        let other_back_colour = if value.change_background {
            colour_change_type = Change::Bg.insert_into(colour_change_type);
            value.background.clone()
        } else {
            String::new()
        };
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
            }
        )
    }
}
