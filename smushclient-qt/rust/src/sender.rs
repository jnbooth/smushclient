use std::time::Duration;

use cxx_qt_lib::{QString, QTime};
use smushclient_plugins::{
    Alias, NaiveTime, Occurrence, Reaction, RegexError, Sender, Timelike, Timer, Trigger,
};

use crate::convert::Convert;
use crate::ffi::{self, QColor};

const MILLISECONDS_PER_SECOND: i32 = 1000;
const SECONDS_PER_MINUTE: u64 = 60;
const MINUTES_PER_HOUR: u64 = 60;

#[derive(Default)]
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

impl SenderRust {
    pub fn populate(&mut self, sender: &Sender) {
        self.send_to = sender.send_to.into();
        self.label = QString::from(&sender.label);
        self.script = QString::from(&sender.script);
        self.group = QString::from(&sender.group);
        self.variable = QString::from(&sender.variable);
        self.text = QString::from(&sender.text);
        self.enabled = sender.enabled;
        self.one_shot = sender.one_shot;
        self.temporary = sender.temporary;
        self.omit_from_output = sender.omit_from_output;
        self.omit_from_log = sender.omit_from_log;
    }
}

impl From<&SenderRust> for Sender {
    fn from(value: &SenderRust) -> Self {
        Self {
            send_to: value.send_to.into(),
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
}

impl_deref!(TimerRust, SenderRust, send);

impl TimerRust {
    pub fn populate(&mut self, timer: &Timer) {
        self.send.populate(&timer.send);
        match timer.occurrence {
            Occurrence::Time(time) => {
                self.occurrence = ffi::Occurrence::Time;
                let seconds = i32::try_from(time.num_seconds_from_midnight()).unwrap();
                let msecs = seconds * MILLISECONDS_PER_SECOND;
                self.at_time = QTime::from_msecs_since_start_of_day(msecs);
            }
            Occurrence::Interval(duration) => {
                self.occurrence = ffi::Occurrence::Interval;
                let seconds = duration.as_secs();
                self.every_second = i32::try_from(seconds % SECONDS_PER_MINUTE).unwrap();
                let minutes = seconds / SECONDS_PER_MINUTE;
                self.every_minute = i32::try_from(minutes % MINUTES_PER_HOUR).unwrap();
                self.every_hour = i32::try_from(minutes / MINUTES_PER_HOUR).unwrap();
            }
        }
        self.active_closed = timer.active_closed;
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
        }
    }
}

#[derive(Default)]
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

impl ReactionRust {
    pub fn populate(&mut self, reaction: &Reaction) {
        self.send.populate(&reaction.send);
        self.sequence = i32::from(reaction.sequence);
        self.pattern = QString::from(&reaction.pattern);
        self.ignore_case = reaction.ignore_case;
        self.keep_evaluating = reaction.keep_evaluating;
        self.is_regex = reaction.is_regex;
        self.expand_variables = reaction.expand_variables;
        self.repeats = reaction.repeats;
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

impl AliasRust {
    pub fn populate(&mut self, alias: &Alias) {
        self.reaction.populate(&alias.reaction);
        self.echo_alias = alias.echo_alias;
        self.menu = alias.menu;
        self.omit_from_command_history = alias.omit_from_command_history;
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

#[derive(Default)]
pub struct TriggerRust {
    pub reaction: ReactionRust,
    pub change_foreground: bool,
    pub foreground: QString,
    pub foreground_color: QColor,
    pub change_background: bool,
    pub background: QString,
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

impl TriggerRust {
    pub fn populate(&mut self, trigger: &Trigger) {
        self.reaction.populate(&trigger.reaction);
        self.change_foreground = trigger.change_foreground;
        self.foreground = QString::from(&trigger.foreground);
        self.foreground_color = trigger.foreground_color.convert();
        self.change_background = trigger.change_background;
        self.background = QString::from(&trigger.background);
        self.background_color = trigger.background_color.convert();
        self.make_bold = trigger.make_bold;
        self.make_italic = trigger.make_italic;
        self.make_underline = trigger.make_underline;
        self.sound = QString::from(&trigger.sound);
        self.sound_if_inactive = trigger.sound_if_inactive;
        self.lowercase_wildcard = trigger.lowercase_wildcard;
        self.multi_line = trigger.multi_line;
        self.lines_to_match = i32::from(trigger.lines_to_match);
    }
}

impl TryFrom<&TriggerRust> for Trigger {
    type Error = RegexError;

    fn try_from(value: &TriggerRust) -> Result<Self, Self::Error> {
        Ok(Self {
            reaction: Reaction::try_from(&value.reaction)?,
            change_foreground: value.change_foreground,
            foreground: String::from(&value.foreground),
            foreground_color: value.foreground_color.convert(),
            change_background: value.change_background,
            background: String::from(&value.background),
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
