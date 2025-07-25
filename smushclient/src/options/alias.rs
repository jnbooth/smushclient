use smushclient_plugins::Alias;

use super::property::BoolProperty;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum AliasBool {
    // Sender
    Enabled,
    OneShot,
    Temporary,
    OmitFromOutput,
    OmitFromLog,
    // Reaction
    IgnoreCase,
    KeepEvaluating,
    IsRegex,
    ExpandVariables,
    Repeats,
    // Alias
    EchoAlias,
    Menu,
    OmitFromCommandHistory,
}

impl BoolProperty for AliasBool {
    type Target = Alias;

    fn get(self, alias: &Alias) -> bool {
        match self {
            Self::Enabled => alias.enabled,
            Self::OneShot => alias.one_shot,
            Self::Temporary => alias.temporary,
            Self::OmitFromOutput => alias.omit_from_output,
            Self::OmitFromLog => alias.omit_from_log,
            Self::IgnoreCase => alias.ignore_case,
            Self::KeepEvaluating => alias.keep_evaluating,
            Self::IsRegex => alias.is_regex,
            Self::ExpandVariables => alias.expand_variables,
            Self::Repeats => alias.repeats,
            Self::EchoAlias => alias.echo_alias,
            Self::Menu => alias.menu,
            Self::OmitFromCommandHistory => alias.omit_from_command_history,
        }
    }

    fn get_mut(self, alias: &mut Alias) -> &mut bool {
        match self {
            Self::Enabled => &mut alias.enabled,
            Self::OneShot => &mut alias.one_shot,
            Self::Temporary => &mut alias.temporary,
            Self::OmitFromOutput => &mut alias.omit_from_output,
            Self::OmitFromLog => &mut alias.omit_from_log,
            Self::IgnoreCase => &mut alias.ignore_case,
            Self::KeepEvaluating => &mut alias.keep_evaluating,
            Self::IsRegex => &mut alias.is_regex,
            Self::ExpandVariables => &mut alias.expand_variables,
            Self::Repeats => &mut alias.repeats,
            Self::EchoAlias => &mut alias.echo_alias,
            Self::Menu => &mut alias.menu,
            Self::OmitFromCommandHistory => &mut alias.omit_from_command_history,
        }
    }
}
