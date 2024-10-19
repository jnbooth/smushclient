use super::property::BoolProperty;
use smushclient_plugins::Trigger;

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum TriggerBool {
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
    // Trigger
    ChangeForeground,
    ChangeBackground,
    MakeBold,
    MakeItalic,
    MakeUnderline,
    SoundIfInactive,
    LowercaseWildcard,
    MultiLine,
}

impl BoolProperty for TriggerBool {
    type Target = Trigger;

    fn get(self, trigger: &Trigger) -> bool {
        match self {
            Self::Enabled => trigger.enabled,
            Self::OneShot => trigger.one_shot,
            Self::Temporary => trigger.temporary,
            Self::OmitFromOutput => trigger.omit_from_output,
            Self::OmitFromLog => trigger.omit_from_log,
            Self::IgnoreCase => trigger.ignore_case,
            Self::KeepEvaluating => trigger.keep_evaluating,
            Self::IsRegex => trigger.is_regex,
            Self::ExpandVariables => trigger.expand_variables,
            Self::Repeats => trigger.repeats,
            Self::ChangeForeground => trigger.change_foreground,
            Self::ChangeBackground => trigger.change_background,
            Self::MakeBold => trigger.make_bold,
            Self::MakeItalic => trigger.make_italic,
            Self::MakeUnderline => trigger.make_underline,
            Self::SoundIfInactive => trigger.sound_if_inactive,
            Self::LowercaseWildcard => trigger.lowercase_wildcard,
            Self::MultiLine => trigger.multi_line,
        }
    }

    fn get_mut(self, trigger: &mut Trigger) -> &mut bool {
        match self {
            Self::Enabled => &mut trigger.enabled,
            Self::OneShot => &mut trigger.one_shot,
            Self::Temporary => &mut trigger.temporary,
            Self::OmitFromOutput => &mut trigger.omit_from_output,
            Self::OmitFromLog => &mut trigger.omit_from_log,
            Self::IgnoreCase => &mut trigger.ignore_case,
            Self::KeepEvaluating => &mut trigger.keep_evaluating,
            Self::IsRegex => &mut trigger.is_regex,
            Self::ExpandVariables => &mut trigger.expand_variables,
            Self::Repeats => &mut trigger.repeats,
            Self::ChangeForeground => &mut trigger.change_foreground,
            Self::ChangeBackground => &mut trigger.change_background,
            Self::MakeBold => &mut trigger.make_bold,
            Self::MakeItalic => &mut trigger.make_italic,
            Self::MakeUnderline => &mut trigger.make_underline,
            Self::SoundIfInactive => &mut trigger.sound_if_inactive,
            Self::LowercaseWildcard => &mut trigger.lowercase_wildcard,
            Self::MultiLine => &mut trigger.multi_line,
        }
    }
}
