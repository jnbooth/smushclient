use std::pin::Pin;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use cxx_qt::{Constructor, Initialize};
use cxx_qt_extensions::QUuid;
use cxx_qt_lib::{QColor, QString, QTime};
use smushclient_plugins::{Alias, Occurrence, Reaction, RegexError, Sender, Timer, Trigger};

use crate::convert::Convert;
use crate::ffi;

const MILLISECONDS_PER_SECOND: i32 = 1000;
const SECONDS_PER_MINUTE: u64 = 60;
const MINUTES_PER_HOUR: u64 = 60;

pub struct SenderRust {
    pub send_to: ffi::SendTarget,
    pub label: QString,
    pub script: QString,
    pub group: QString,
    pub variable: QString,
    pub text: QString,

    pub enabled: bool,
    pub one_shot: bool,
    pub temporary: bool,
    pub omit_from_output: bool,
    pub omit_from_log: bool,
}

impl Default for SenderRust {
    fn default() -> Self {
        Self {
            text: QString::default(),
            send_to: ffi::SendTarget::World,
            label: QString::default(),
            script: QString::default(),
            group: QString::default(),
            variable: QString::default(),
            enabled: true,
            one_shot: false,
            temporary: false,
            omit_from_output: false,
            omit_from_log: false,
        }
    }
}

impl From<&Sender> for SenderRust {
    fn from(sender: &Sender) -> Self {
        Self {
            send_to: sender.send_to.into(),
            label: QString::from(&sender.label),
            script: QString::from(&sender.script),
            group: QString::from(&sender.group),
            variable: QString::from(&sender.variable),
            text: QString::from(&sender.text),
            enabled: sender.enabled,
            one_shot: sender.one_shot,
            temporary: sender.temporary,
            omit_from_output: sender.omit_from_output,
            omit_from_log: sender.omit_from_log,
        }
    }
}

impl From<&SenderRust> for Sender {
    fn from(value: &SenderRust) -> Self {
        Self {
            send_to: value.send_to.try_into().unwrap_or_default(),
            label: String::from(&value.label),
            script: String::from(&value.script),
            group: String::from(&value.group),
            variable: String::from(&value.variable),
            text: String::from(&value.text),
            enabled: value.enabled,
            one_shot: value.one_shot,
            temporary: value.temporary,
            omit_from_output: value.omit_from_output,
            omit_from_log: value.omit_from_log,
        }
    }
}

#[derive(Default)]
pub struct TimerRust {
    pub send: SenderRust,
    pub occurrence: ffi::Occurrence,
    pub at_time: QTime,
    pub every_hour: i32,
    pub every_minute: i32,
    pub every_second: i32,
    pub active_closed: bool,
    pub id: QUuid,
}

impl_deref!(TimerRust, SenderRust, send);

impl From<&Timer> for TimerRust {
    fn from(timer: &Timer) -> Self {
        let send = SenderRust::from(&timer.send);

        match timer.occurrence {
            Occurrence::Time(time) => {
                let seconds = i32::try_from(time.num_seconds_from_midnight()).unwrap();
                let msecs = seconds * MILLISECONDS_PER_SECOND;
                Self {
                    send,
                    occurrence: ffi::Occurrence::Time,
                    at_time: QTime::from_msecs_since_start_of_day(msecs),
                    every_second: 0,
                    every_minute: 0,
                    every_hour: 0,
                    active_closed: timer.active_closed,
                    id: timer.id.into(),
                }
            }
            Occurrence::Interval(duration) => {
                let seconds = duration.as_secs();
                let minutes = seconds / SECONDS_PER_MINUTE;
                Self {
                    send,
                    occurrence: ffi::Occurrence::Interval,
                    at_time: QTime::default(),
                    every_second: i32::try_from(seconds % SECONDS_PER_MINUTE).unwrap(),
                    every_minute: i32::try_from(minutes % MINUTES_PER_HOUR).unwrap(),
                    every_hour: i32::try_from(minutes / MINUTES_PER_HOUR).unwrap(),
                    active_closed: timer.active_closed,
                    id: timer.id.into(),
                }
            }
        }
    }
}

impl From<&TimerRust> for Timer {
    fn from(value: &TimerRust) -> Self {
        let occurrence = match value.occurrence {
            ffi::Occurrence::Time => {
                let msecs = value.at_time.msecs_since_start_of_day();
                let seconds = u32::try_from(msecs / MILLISECONDS_PER_SECOND).unwrap();
                let time = NaiveTime::from_num_seconds_from_midnight_opt(seconds, 0).unwrap();
                Occurrence::Time(time)
            }
            ffi::Occurrence::Interval => {
                let seconds = u64::try_from(value.every_second).unwrap();
                let minutes = u64::try_from(value.every_minute).unwrap();
                let hours = u64::try_from(value.every_hour).unwrap();
                let duration = seconds + SECONDS_PER_MINUTE * (minutes + MINUTES_PER_HOUR * hours);
                Occurrence::Interval(Duration::from_secs(duration))
            }
            _ => unreachable!(),
        };
        Self {
            send: Sender::from(&value.send),
            occurrence,
            active_closed: value.active_closed,
            id: value.id.into(),
        }
    }
}

pub struct ReactionRust {
    pub sequence: i32,
    pub pattern: QString,
    pub send: SenderRust,

    pub ignore_case: bool,
    pub keep_evaluating: bool,
    pub is_regex: bool,
    pub expand_variables: bool,
    pub repeats: bool,
}

impl_deref!(ReactionRust, SenderRust, send);

impl Default for ReactionRust {
    fn default() -> Self {
        Self {
            sequence: i32::from(Reaction::DEFAULT_SEQUENCE),
            pattern: QString::default(),
            send: SenderRust::default(),
            ignore_case: false,
            keep_evaluating: false,
            is_regex: false,
            expand_variables: false,
            repeats: false,
        }
    }
}

impl From<&Reaction> for ReactionRust {
    fn from(reaction: &Reaction) -> Self {
        Self {
            sequence: i32::from(reaction.sequence),
            send: SenderRust::from(&reaction.send),
            pattern: QString::from(&reaction.pattern),
            ignore_case: reaction.ignore_case,
            keep_evaluating: reaction.keep_evaluating,
            is_regex: reaction.is_regex,
            expand_variables: reaction.expand_variables,
            repeats: reaction.repeats,
        }
    }
}

impl TryFrom<&ReactionRust> for Reaction {
    type Error = RegexError;

    fn try_from(value: &ReactionRust) -> Result<Self, Self::Error> {
        let pattern = String::from(&value.pattern);
        let regex = Reaction::make_regex(&pattern, value.is_regex)?;
        Ok(Self {
            sequence: i16::try_from(value.sequence).unwrap(),
            pattern,
            send: Sender::from(&value.send),
            ignore_case: value.ignore_case,
            keep_evaluating: value.keep_evaluating,
            is_regex: value.is_regex,
            expand_variables: value.expand_variables,
            repeats: value.repeats,
            regex,
        })
    }
}

#[derive(Default)]
pub struct AliasRust {
    pub reaction: ReactionRust,
    pub echo_alias: bool,
    pub menu: bool,
    pub omit_from_command_history: bool,
}

impl_deref!(AliasRust, ReactionRust, reaction);

impl From<&Alias> for AliasRust {
    fn from(alias: &Alias) -> Self {
        Self {
            reaction: ReactionRust::from(&alias.reaction),
            echo_alias: alias.echo_alias,
            menu: alias.menu,
            omit_from_command_history: alias.omit_from_command_history,
        }
    }
}

impl TryFrom<&AliasRust> for Alias {
    type Error = RegexError;

    fn try_from(value: &AliasRust) -> Result<Self, Self::Error> {
        Ok(Self {
            reaction: Reaction::try_from(&value.reaction)?,
            echo_alias: value.echo_alias,
            menu: value.menu,
            omit_from_command_history: value.omit_from_command_history,
        })
    }
}

pub struct TriggerRust {
    pub reaction: ReactionRust,
    pub change_foreground: bool,
    pub foreground_color: QColor,
    pub change_background: bool,
    pub background_color: QColor,
    pub make_bold: bool,
    pub make_italic: bool,
    pub make_underline: bool,
    pub sound: QString,
    pub sound_if_inactive: bool,
    pub lowercase_wildcard: bool,
    pub multi_line: bool,
    pub lines_to_match: i32,
}

impl_deref!(TriggerRust, ReactionRust, reaction);

impl Default for TriggerRust {
    fn default() -> Self {
        Self {
            reaction: ReactionRust::default(),
            change_foreground: false,
            foreground_color: QColor::from_rgb(255, 255, 255),
            change_background: false,
            background_color: QColor::from_rgb(0, 0, 0),
            make_bold: false,
            make_italic: false,
            make_underline: false,
            sound: QString::default(),
            sound_if_inactive: false,
            lowercase_wildcard: false,
            multi_line: false,
            lines_to_match: 0,
        }
    }
}

impl From<&Trigger> for TriggerRust {
    fn from(trigger: &Trigger) -> Self {
        Self {
            reaction: ReactionRust::from(&trigger.reaction),
            change_foreground: trigger.change_foreground,
            foreground_color: trigger.foreground_color.convert(),
            change_background: trigger.change_background,
            background_color: trigger.background_color.convert(),
            make_bold: trigger.make_bold,
            make_italic: trigger.make_italic,
            make_underline: trigger.make_underline,
            sound: QString::from(&trigger.sound),
            sound_if_inactive: trigger.sound_if_inactive,
            lowercase_wildcard: trigger.lowercase_wildcard,
            multi_line: trigger.multi_line,
            lines_to_match: i32::from(trigger.lines_to_match),
        }
    }
}

impl TryFrom<&TriggerRust> for Trigger {
    type Error = RegexError;

    fn try_from(value: &TriggerRust) -> Result<Self, Self::Error> {
        Ok(Self {
            reaction: Reaction::try_from(&value.reaction)?,
            change_foreground: value.change_foreground,
            foreground_color: value.foreground_color.convert(),
            change_background: value.change_background,
            background_color: value.background_color.convert(),
            make_bold: value.make_bold,
            make_italic: value.make_italic,
            make_underline: value.make_underline,
            sound: String::from(&value.sound),
            sound_if_inactive: value.sound_if_inactive,
            lowercase_wildcard: value.lowercase_wildcard,
            multi_line: value.multi_line,
            lines_to_match: u8::try_from(value.lines_to_match).unwrap(),
        })
    }
}

impl Initialize for ffi::Alias {
    fn initialize(self: Pin<&mut Self>) {}
}

impl Initialize for ffi::Timer {
    fn initialize(self: Pin<&mut Self>) {}
}

impl Initialize for ffi::Trigger {
    fn initialize(self: Pin<&mut Self>) {}
}

impl Constructor<(*const ffi::World, usize)> for ffi::Alias {
    type BaseArguments = ();
    type InitializeArguments = ();
    type NewArguments = (*const ffi::World, usize);

    fn route_arguments(
        args: Self::NewArguments,
    ) -> (
        Self::NewArguments,
        Self::BaseArguments,
        Self::InitializeArguments,
    ) {
        (args, (), ())
    }

    fn new(args: (*const ffi::World, usize)) -> AliasRust {
        match unsafe { (*args.0).cxx_qt_ffi_rust().aliases.get(args.1) } {
            Some(alias) => AliasRust::from(alias),
            None => AliasRust::default(),
        }
    }
}

impl Constructor<(*const ffi::World, usize)> for ffi::Timer {
    type BaseArguments = ();
    type InitializeArguments = ();
    type NewArguments = (*const ffi::World, usize);

    fn route_arguments(
        args: Self::NewArguments,
    ) -> (
        Self::NewArguments,
        Self::BaseArguments,
        Self::InitializeArguments,
    ) {
        (args, (), ())
    }

    fn new(args: (*const ffi::World, usize)) -> TimerRust {
        match unsafe { (*args.0).cxx_qt_ffi_rust().timers.get(args.1) } {
            Some(timer) => TimerRust::from(timer),
            None => TimerRust::default(),
        }
    }
}

impl Constructor<(*const ffi::World, usize)> for ffi::Trigger {
    type BaseArguments = ();
    type InitializeArguments = ();
    type NewArguments = (*const ffi::World, usize);

    fn route_arguments(
        args: Self::NewArguments,
    ) -> (
        Self::NewArguments,
        Self::BaseArguments,
        Self::InitializeArguments,
    ) {
        (args, (), ())
    }

    fn new(args: (*const ffi::World, usize)) -> TriggerRust {
        match unsafe { (*args.0).cxx_qt_ffi_rust().triggers.get(args.1) } {
            Some(trigger) => TriggerRust::from(trigger),
            None => TriggerRust::default(),
        }
    }
}
