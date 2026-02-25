use std::borrow::Cow;

use serde::{Deserialize, Serialize};

use super::reaction::Reaction;
use super::send_to::{SendTarget, sendto_serde};
use super::sender::Sender;
use crate::xml::{XmlIterable, bool_serde, is_default};

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
    type Xml<'a> = XmlAlias<'a>;
}

#[derive(Debug, Default, Deserialize, Serialize)]
#[serde(rename = "alias", default = "XmlAlias::template")]
#[rustfmt::skip]
pub struct XmlAlias<'a> {
    #[serde(rename = "@echo_alias", with = "bool_serde", skip_serializing_if = "is_default")]
    pub echo_alias: bool,
    #[serde(rename = "@enabled", with = "bool_serde")]
    pub enabled: bool,
    #[serde(rename = "@expand_variables", with = "bool_serde", skip_serializing_if = "is_default")]
    pub expand_variables: bool,
    #[serde(rename = "@group", borrow, skip_serializing_if = "is_default")]
    pub group: Cow<'a, str>,
    #[serde(rename = "@ignore_case", with = "bool_serde", skip_serializing_if = "is_default")]
    pub ignore_case: bool,
    #[serde(rename = "@keep_evaluating", with = "bool_serde", skip_serializing_if = "is_default")]
    pub keep_evaluating: bool,
    #[serde(rename = "@match", borrow, skip_serializing_if = "is_default")]
    pub pattern: Cow<'a, str>,
    #[serde(rename = "@menu", with = "bool_serde", skip_serializing_if = "is_default")]
    pub menu: bool,
    #[serde(rename = "@name", borrow, skip_serializing_if = "is_default")]
    pub name: Cow<'a, str>,
    #[serde(rename = "@omit_from_command_history", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_command_history: bool,
    #[serde(rename = "@omit_from_log", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_log: bool,
    #[serde(rename = "@omit_from_output", with = "bool_serde", skip_serializing_if = "is_default")]
    pub omit_from_output: bool,
    #[serde(rename = "@one_shot", with = "bool_serde", skip_serializing_if = "is_default")]
    pub one_shot: bool,
    #[serde(rename = "@regexp", with = "bool_serde", skip_serializing_if = "is_default")]
    pub regexp: bool,
    #[serde(rename = "@script", borrow, skip_serializing_if = "is_default")]
    pub script: Cow<'a, str>,
    #[serde(borrow, skip_serializing_if = "is_default")]
    pub send: Cow<'a, str>,
    #[serde(rename = "@send_to", with = "sendto_serde", skip_serializing_if = "is_default")]
    pub send_to: SendTarget,
    #[serde(rename = "@sequence")]
    pub sequence: i16,
    #[serde(rename = "@temporary", with = "bool_serde", skip_serializing_if = "is_default")]
    pub temporary: bool,
    #[serde(rename = "@user", skip_serializing_if = "is_default")]
    pub user: i64,
    #[serde(rename = "@variable", borrow, skip_serializing_if = "is_default")]
    pub variable: Cow<'a, str>,
}
impl XmlAlias<'_> {
    fn template() -> Self {
        Self {
            enabled: true,
            sequence: Reaction::DEFAULT_SEQUENCE,
            ..Default::default()
        }
    }
}
impl TryFrom<XmlAlias<'_>> for Alias {
    type Error = crate::regex::RegexError;

    fn try_from(value: XmlAlias) -> Result<Self, Self::Error> {
        let regex = Reaction::make_regex(&value.pattern, value.regexp, value.ignore_case)?;
        Ok(Self {
            echo_alias: value.echo_alias,
            menu: value.menu,
            omit_from_command_history: value.omit_from_command_history,
            reaction: Reaction {
                sequence: value.sequence,
                pattern: value.pattern.into(),
                ignore_case: value.ignore_case,
                keep_evaluating: value.keep_evaluating,
                is_regex: value.regexp,
                expand_variables: value.expand_variables,
                repeats: false,
                regex: regex.into(),
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
            },
        })
    }
}
impl<'a> From<&'a Alias> for XmlAlias<'a> {
    fn from(value: &'a Alias) -> Self {
        Self {
            echo_alias: value.echo_alias,
            enabled: value.enabled,
            expand_variables: value.expand_variables,
            group: (&value.group).into(),
            ignore_case: value.ignore_case,
            keep_evaluating: value.keep_evaluating,
            menu: value.menu,
            name: (&value.label).into(),
            omit_from_command_history: value.omit_from_command_history,
            omit_from_log: value.omit_from_log,
            omit_from_output: value.omit_from_output,
            one_shot: value.one_shot,
            pattern: (&value.pattern).into(),
            regexp: value.is_regex,
            script: (&value.script).into(),
            send_to: value.send_to,
            send: (&value.text).into(),
            sequence: value.sequence,
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
        let alias = Alias::default();
        let xml = alias.to_xml_string().expect("error serializing alias");
        assert_eq!(xml, r#"<alias enabled="y" sequence="100"></alias>"#);
        let mut roundtrip = Alias::from_xml_str(&xml).expect("error deserializing alias");
        roundtrip.id = alias.id;
        assert_eq!(roundtrip, alias);
    }
}
