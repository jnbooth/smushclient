use std::borrow::Cow;

use serde::{Deserialize, Serialize};

use super::reaction::Reaction;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::in_place::{InPlace, in_place};
use crate::xml::{XmlIterable, bool_serde};

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
pub struct Alias {
    // Note: This is at the top for Ord-deriving purposes.
    pub reaction: Reaction,
    pub echo_alias: bool,
    pub menu: bool,
    pub omit_from_command_history: bool,
}

impl_deref!(Alias, Reaction, reaction);
impl_asref!(Alias, Reaction);
impl_asref!(Alias, Sender);

impl XmlIterable for Alias {
    const TAG: &'static str = "alias";
    type Xml<'a> = AliasXml<'a>;
}

#[derive(Debug, Default, Deserialize, Serialize)]
#[serde(default = "AliasXml::template")]
#[rustfmt::skip]
pub struct AliasXml<'a> {
    #[serde(rename = "@name", borrow, default, skip_serializing_if = "str::is_empty")]
    label: Cow<'a, str>,
    #[serde(rename = "@script", borrow, default, skip_serializing_if = "str::is_empty")]
    script: Cow<'a, str>,
    #[serde(rename = "@match", borrow, default, skip_serializing_if = "str::is_empty")]
    pattern: Cow<'a, str>,
    #[serde(rename = "@enabled", with = "bool_serde")]
    enabled: bool,
    #[serde(rename = "@echo_alias", with = "bool_serde")]
    echo_alias: bool,
    #[serde(rename = "@expand_variables", with = "bool_serde")]
    expand_variables: bool,
    #[serde(rename = "@group", borrow, default, skip_serializing_if = "str::is_empty")]
    group: Cow<'a, str>,
    #[serde(rename = "@variable", borrow, default, skip_serializing_if = "str::is_empty")]
    variable: Cow<'a, str>,
    #[serde(rename = "@omit_from_command_history", with = "bool_serde")]
    omit_from_command_history: bool,
    #[serde(rename = "@omit_from_log", with = "bool_serde")]
    omit_from_log: bool,
    #[serde(rename = "@regexp", with = "bool_serde")]
    is_regex: bool,
    #[serde(with = "sendto_serde", rename = "@send_to")]
    send_to: SendTarget,
    #[serde(rename = "@omit_from_output", with = "bool_serde")]
    omit_from_output: bool,
    #[serde(rename = "@one_shot", with = "bool_serde")]
    one_shot: bool,
    #[serde(rename = "@menu", with = "bool_serde")]
    menu: bool,
    #[serde(rename = "@ignore_case", with = "bool_serde")]
    ignore_case: bool,
    #[serde(rename = "@keep_evaluating", with = "bool_serde")]
    keep_evaluating: bool,
    #[serde(rename = "@sequence")]
    sequence: i16,
    #[serde(rename = "@temporary", with = "bool_serde")]
    temporary: bool,
    #[serde(borrow, default, rename = "send")]
    text: Vec<Cow<'a, str>>,
}
impl AliasXml<'_> {
    fn template() -> Self {
        Self {
            enabled: true,
            sequence: Reaction::DEFAULT_SEQUENCE,
            ..Default::default()
        }
    }
}
impl TryFrom<AliasXml<'_>> for Alias {
    type Error = crate::regex::RegexError;

    fn try_from(value: AliasXml) -> Result<Self, Self::Error> {
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
                repeats: false,
                ..sequence,
                ..pattern,
                ..is_regex,
                ..ignore_case,
                ..keep_evaluating,
                ..expand_variables,
            }
        );
        Ok(in_place!(
            value,
            Self {
                reaction,
                ..echo_alias,
                ..menu,
                ..omit_from_command_history,
            }
        ))
    }
}
impl<'a> From<&'a Alias> for AliasXml<'a> {
    fn from(value: &'a Alias) -> Self {
        in_place!(
            value,
            Self {
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
                ..echo_alias,
                ..menu,
                ..omit_from_command_history,
            }
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn xml_roundtrip() {
        let alias = Alias::default();
        let to_xml =
            quick_xml::se::to_string(&AliasXml::from(&alias)).expect("error serializing alias");
        let from_xml: AliasXml =
            quick_xml::de::from_str(&to_xml).expect("error deserializing alias");
        let roundtrip = Alias::try_from(from_xml).expect("error converting alias");
        assert_eq!(roundtrip, alias);
    }
}
