use std::borrow::Cow;

use flagset::{FlagSet, flags};
use mxp::RgbColor;
use serde::{Deserialize, Serialize};

use super::reaction::Reaction;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::xml::{XmlIterable, bool_serde, is_default};

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
    type Xml<'a> = XmlTrigger<'a>;
}

flags! {
    #[derive(PartialOrd, Ord, Hash)]
    enum Change: u8 {
        Both,
        Fg,
        Bg,
    }
}

#[derive(Debug, Default, Deserialize, Serialize)]
#[serde(rename = "trigger", default = "XmlTrigger::template")]
#[rustfmt::skip]
pub struct XmlTrigger<'a> {
    // pub back_colour: Option<usize>,
    // pub bold: bool,
    #[serde(rename = "@clipboard_arg", skip_serializing_if = "is_default")]
    pub clipboard_arg: u8,
    #[serde(rename = "@color_change_type", skip_serializing_if = "is_default")]
    pub colour_change_type: u8,
    #[serde(rename = "@custom_colour", skip_serializing_if = "is_default")]
    pub custom_colour: usize,
    #[serde(rename = "@enabled", with = "bool_serde")]
    pub enabled: bool,
    #[serde(rename = "@expand_variables", with = "bool_serde", skip_serializing_if = "is_default")]
    pub expand_variables: bool,
    #[serde(rename = "@group", borrow, skip_serializing_if = "is_default")]
    pub group: Cow<'a, str>,
    #[serde(rename = "@ignore_case", with = "bool_serde", skip_serializing_if = "is_default")]
    pub ignore_case: bool,
    // pub inverse: bool,
    // pub italic: bool,
    #[serde(rename = "@keep_evaluating", with = "bool_serde", skip_serializing_if = "is_default")]
    pub keep_evaluating: bool,
    #[serde(rename = "@lines_to_match", skip_serializing_if = "is_default")]
    pub lines_to_match: u8,
    #[serde(rename = "@lowercase_wildcard", with = "bool_serde", skip_serializing_if = "is_default")]
    pub lowercase_wildcard: bool,
    #[serde(rename = "@make_bold", with = "bool_serde", skip_serializing_if = "is_default")]
    pub make_bold: bool,
    #[serde(rename = "@make_italic", with = "bool_serde", skip_serializing_if = "is_default")]
    pub make_italic: bool,
    #[serde(rename = "@make_underline", with = "bool_serde", skip_serializing_if = "is_default")]
    pub make_underline: bool,
    #[serde(rename = "@match", borrow, skip_serializing_if = "is_default")]
    pub pattern: Cow<'a, str>,
    // pub match_back_colour: bool,
    // pub match_bold: bool,
    // pub match_inverse: bool,
    // pub match_italic: bool,
    // pub match_text_colour: bool,
    // pub match_underline: bool,
    #[serde(rename = "@multi_line", with = "bool_serde", skip_serializing_if = "is_default")]
    pub multi_line: bool,
    #[serde(rename = "@name", borrow, skip_serializing_if = "is_default")]
    pub name: Cow<'a, str>,
    #[serde(rename = "@omit_from_log", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_log: bool,
    #[serde(rename = "@omit_from_output", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_output: bool,
    #[serde(rename = "@one_shot", with = "bool_serde", skip_serializing_if = "is_default")]
    pub one_shot: bool,
    #[serde(rename = "@other_back_colour", skip_serializing_if = "is_default")]
    pub other_back_colour: Option<RgbColor>,
    #[serde(rename = "@other_text_colour", skip_serializing_if = "is_default")]
    pub other_text_colour: Option<RgbColor>,
    #[serde(rename = "@regexp", with = "bool_serde", skip_serializing_if = "is_default")]
    pub regexp: bool,
    #[serde(rename = "@repeat", with = "bool_serde", skip_serializing_if = "is_default")]
    pub repeat: bool,
    #[serde(rename = "@script", borrow, skip_serializing_if = "is_default")]
    pub script: Cow<'a, str>,
    #[serde(rename = "@send_to", with = "sendto_serde", skip_serializing_if = "is_default")]
    pub send_to: SendTarget,
    #[serde(rename = "@sequence")]
    pub sequence: i16,
    #[serde(rename = "@sound", borrow, skip_serializing_if = "is_default")]
    pub sound: Cow<'a, str>,
    #[serde(rename = "@sound_if_inactive", with = "bool_serde", skip_serializing_if = "is_default")]
    pub sound_if_inactive: bool,
    #[serde(rename = "@temporary", with = "bool_serde", skip_serializing_if = "is_default")]
    pub temporary: bool,
    #[serde(borrow, rename = "send", skip_serializing_if = "is_default")]
    pub text: Cow<'a, str>,
    // text_colour: Option<usize>,
    #[serde(rename = "@user", skip_serializing_if = "is_default")]
    pub user: i64,
    #[serde(rename = "@variable", borrow, skip_serializing_if = "is_default")]
    pub variable: Cow<'a, str>,
}
impl XmlTrigger<'_> {
    fn template() -> Self {
        Self {
            enabled: true,
            sequence: Reaction::DEFAULT_SEQUENCE,
            ..Default::default()
        }
    }
}
impl TryFrom<XmlTrigger<'_>> for Trigger {
    type Error = crate::regex::RegexError;

    fn try_from(value: XmlTrigger) -> Result<Self, Self::Error> {
        let regex = Reaction::make_regex(&value.pattern, value.regexp, value.ignore_case)?;
        let color_changes = FlagSet::new_truncated(value.colour_change_type);

        Ok(Self {
            change_foreground: color_changes.contains(Change::Fg),
            foreground_color: value.other_text_colour.unwrap_or(RgbColor::WHITE),
            change_background: color_changes.contains(Change::Bg),
            background_color: value.other_back_colour.unwrap_or(RgbColor::BLACK),
            sound: value.sound.into(),
            make_bold: value.make_bold,
            make_italic: value.make_italic,
            make_underline: value.make_underline,
            sound_if_inactive: value.sound_if_inactive,
            lowercase_wildcard: value.lowercase_wildcard,
            multi_line: value.multi_line,
            lines_to_match: value.lines_to_match,
            clipboard_arg: value.clipboard_arg,
            reaction: Reaction {
                sequence: value.sequence,
                pattern: value.pattern.into(),
                ignore_case: value.ignore_case,
                keep_evaluating: value.keep_evaluating,
                is_regex: value.regexp,
                expand_variables: value.expand_variables,
                repeats: value.repeat,
                regex: regex.into(),
                send: Sender {
                    group: value.group.into(),
                    label: value.name.into(),
                    send_to: value.send_to,
                    script: value.script.into(),
                    variable: value.variable.into(),
                    text: value.text.into(),
                    enabled: value.enabled,
                    one_shot: value.one_shot,
                    temporary: value.temporary,
                    omit_from_output: value.omit_from_output,
                    omit_from_log: value.omit_from_log,
                    id: Sender::get_id(),
                    userdata: value.user,
                },
            },
        })
    }
}
impl<'a> From<&'a Trigger> for XmlTrigger<'a> {
    fn from(value: &'a Trigger) -> Self {
        let mut color_changes = FlagSet::empty();
        let other_text_colour = if value.change_foreground {
            color_changes |= Change::Fg;
            Some(value.foreground_color)
        } else {
            None
        };
        let other_back_colour = if value.change_background {
            color_changes |= Change::Bg;
            Some(value.background_color)
        } else {
            None
        };
        Self {
            clipboard_arg: value.clipboard_arg,
            colour_change_type: color_changes.bits(),
            custom_colour: 0,
            enabled: value.enabled,
            expand_variables: value.expand_variables,
            group: (&value.group).into(),
            ignore_case: value.ignore_case,
            keep_evaluating: value.keep_evaluating,
            lines_to_match: value.lines_to_match,
            lowercase_wildcard: value.lowercase_wildcard,
            make_bold: value.make_bold,
            make_italic: value.make_italic,
            make_underline: value.make_underline,
            multi_line: value.multi_line,
            name: (&value.label).into(),
            omit_from_log: value.omit_from_log,
            omit_from_output: value.omit_from_output,
            one_shot: value.one_shot,
            other_back_colour,
            other_text_colour,
            pattern: (&value.pattern).into(),
            regexp: value.is_regex,
            repeat: value.repeats,
            script: (&value.script).into(),
            send_to: value.send_to,
            sequence: value.sequence,
            sound_if_inactive: value.sound_if_inactive,
            sound: (&value.sound).into(),
            temporary: value.temporary,
            text: (&value.text).into(),
            user: value.userdata,
            variable: (&value.variable).into(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xml_roundtrip() {
        let trigger = Trigger::default();
        let xml = trigger.to_xml_string().expect("error serializing trigger");
        assert_eq!(xml, r#"<trigger enabled="y" sequence="100"></trigger>"#);
        let mut roundtrip = Trigger::from_xml_str(&xml).expect("error deserializing trigger");
        roundtrip.id = trigger.id;
        assert_eq!(roundtrip, trigger);
    }
}
