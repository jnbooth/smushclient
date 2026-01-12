#![allow(clippy::cast_possible_truncation)]
#![allow(clippy::cast_possible_wrap)]
#![allow(clippy::cast_sign_loss)]
use std::num::NonZero;
use std::pin::Pin;
use std::ptr;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use cxx_qt::{CxxQtType, Initialize};
use cxx_qt_lib::{QColor, QString, QTime};
use flagset::{FlagSet, Flags};
use mud_transformer::{Output, OutputFragment, TextFragment, TextStyle};
use smushclient_plugins::{Alias, Occurrence, Reaction, RegexError, Sender, Timer, Trigger};

use crate::convert::{Convert, impl_constructor, impl_deref};
use crate::ffi;

const NANOSECONDS_PER_MILLISECOND: u32 = 1_000_000;
const MILLISECONDS_PER_SECOND: i32 = 1000;
const SECONDS_PER_MINUTE: u64 = 60;
const MINUTES_PER_HOUR: u64 = 60;

impl From<ffi::SendTarget> for ffi::UserSendTarget {
    fn from(value: ffi::SendTarget) -> Self {
        use ffi::SendTarget as T;

        match value {
            T::World | T::WorldDelay | T::Execute | T::Speedwalk | T::WorldImmediate => Self::World,
            T::Command => Self::Command,
            T::Output => Self::Output,
            T::Status => Self::Status,
            T::NotepadNew => Self::NotepadNew,
            T::NotepadAppend => Self::NotepadAppend,
            T::NotepadReplace => Self::NotepadReplace,
            T::Log => Self::Log,
            T::Variable => Self::Variable,
            T::Script | T::ScriptAfterOmit => Self::Script,
            T { repr } => Self { repr },
        }
    }
}

impl From<ffi::UserSendTarget> for ffi::SendTarget {
    fn from(value: ffi::UserSendTarget) -> Self {
        use ffi::UserSendTarget as T;

        match value {
            T::World => Self::World,
            T::Command => Self::Command,
            T::Output => Self::Output,
            T::Status => Self::Status,
            T::NotepadNew => Self::NotepadNew,
            T::NotepadAppend => Self::NotepadAppend,
            T::NotepadReplace => Self::NotepadReplace,
            T::Log => Self::Log,
            T::Variable => Self::Variable,
            T::Script => Self::Script,
            T { repr } => Self { repr },
        }
    }
}

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

    pub id: u16,
    pub userdata: i64,
}

impl Default for SenderRust {
    fn default() -> Self {
        Self::from(&Sender::default())
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
            id: sender.id,
            userdata: sender.userdata,
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
            id: value.id,
            userdata: value.userdata,
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
    pub every_millisecond: u32,
    pub active_closed: bool,
    pub id: u16,
}

impl_deref!(TimerRust, SenderRust, send);

impl From<&Timer> for TimerRust {
    fn from(timer: &Timer) -> Self {
        let send = SenderRust::from(&timer.send);

        match timer.occurrence {
            Occurrence::Time(time) => {
                let seconds = time.num_seconds_from_midnight() as i32;
                let msecs = seconds * MILLISECONDS_PER_SECOND;
                Self {
                    send,
                    occurrence: ffi::Occurrence::Time,
                    at_time: QTime::from_msecs_since_start_of_day(msecs),
                    every_millisecond: 0,
                    every_second: 0,
                    every_minute: 0,
                    every_hour: 0,
                    active_closed: timer.active_closed,
                    id: timer.id,
                }
            }
            Occurrence::Interval(duration) => {
                let seconds = duration.as_secs();
                let minutes = seconds / SECONDS_PER_MINUTE;
                Self {
                    send,
                    occurrence: ffi::Occurrence::Interval,
                    at_time: QTime::default(),
                    every_millisecond: duration.subsec_millis(),
                    every_second: (seconds % SECONDS_PER_MINUTE) as i32,
                    every_minute: (minutes % MINUTES_PER_HOUR) as i32,
                    every_hour: (minutes / MINUTES_PER_HOUR) as i32,
                    active_closed: timer.active_closed,
                    id: timer.id,
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
                let seconds = (msecs / MILLISECONDS_PER_SECOND) as u32;
                let time =
                    NaiveTime::from_num_seconds_from_midnight_opt(seconds, 0).unwrap_or_default();
                Occurrence::Time(time)
            }
            ffi::Occurrence::Interval => {
                let seconds = value.every_second as u64;
                let minutes = value.every_minute as u64;
                let hours = value.every_hour as u64;
                let duration = seconds + SECONDS_PER_MINUTE * (minutes + MINUTES_PER_HOUR * hours);
                Occurrence::Interval(Duration::new(
                    duration,
                    value.every_millisecond * NANOSECONDS_PER_MILLISECOND,
                ))
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
        let regex = Reaction::make_regex(&pattern, value.is_regex, value.ignore_case)?;
        Ok(Self {
            sequence: value.sequence as i16,
            pattern,
            send: Sender::from(&value.send),
            ignore_case: value.ignore_case,
            keep_evaluating: value.keep_evaluating,
            is_regex: value.is_regex,
            expand_variables: value.expand_variables,
            repeats: value.repeats,
            regex: regex.into(),
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
    pub clipboard_arg: i32,
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
            clipboard_arg: 0,
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
            clipboard_arg: match trigger.clipboard_arg {
                Some(n) => i32::from(n.get()),
                None => 0,
            },
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
            lines_to_match: u8::try_from(value.lines_to_match).unwrap_or(u8::MAX),
            clipboard_arg: u8::try_from(value.clipboard_arg)
                .ok()
                .and_then(NonZero::new),
        })
    }
}

impl ffi::Alias {
    pub fn get_user_send_to(&self) -> ffi::UserSendTarget {
        self.send_to.into()
    }

    pub fn set_user_send_to(self: Pin<&mut Self>, send_to: ffi::UserSendTarget) {
        self.rust_mut().send_to = send_to.into();
    }
}

impl ffi::Timer {
    pub fn get_user_send_to(&self) -> ffi::UserSendTarget {
        self.send_to.into()
    }

    pub fn set_user_send_to(self: Pin<&mut Self>, send_to: ffi::UserSendTarget) {
        self.rust_mut().send_to = send_to.into();
    }
}

impl ffi::Trigger {
    pub fn get_user_send_to(&self) -> ffi::UserSendTarget {
        self.send_to.into()
    }

    pub fn set_user_send_to(self: Pin<&mut Self>, send_to: ffi::UserSendTarget) {
        self.rust_mut().send_to = send_to.into();
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

impl_constructor!(<'a>, ffi::Alias, (&'a ffi::SmushClient, usize), {
    fn new((client, i): (&'a ffi::SmushClient, usize)) -> AliasRust {
        match client.borrow_world_sender::<Alias>(i) {
            Some(alias) => AliasRust::from(&*alias),
            None => AliasRust::default(),
        }
    }
});

impl_constructor!(<'a>, ffi::Timer, (&'a ffi::SmushClient, usize), {
    fn new((client, i): (&'a ffi::SmushClient, usize)) -> TimerRust {
        match client.borrow_world_sender::<Timer>(i) {
            Some(timer) => TimerRust::from(&*timer),
            None => TimerRust::default(),
        }
    }
});

impl_constructor!(<'a>, ffi::Trigger, (&'a ffi::SmushClient, usize), {
    fn new((client, i): (&'a ffi::SmushClient, usize)) -> TriggerRust {
        match client.borrow_world_sender::<Trigger>(i) {
            Some(trigger) => TriggerRust::from(&*trigger),
            None => TriggerRust::default(),
        }
    }
});

#[inline(always)]
fn if_contains<E: Flags>(set: FlagSet<E>, value: E, flag: u8) -> u8 {
    if set.contains(value) { flag } else { 0 }
}

#[repr(transparent)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TextSpan {
    inner: TextFragment,
}

impl TextSpan {
    pub fn foreground(&self) -> i32 {
        self.inner.foreground.code() as i32
    }

    pub fn background(&self) -> i32 {
        self.inner.background.code() as i32
    }

    pub fn text(&self) -> &str {
        &self.inner.text
    }

    pub fn style(&self) -> u8 {
        let flags = self.inner.flags;
        if_contains(flags, TextStyle::Bold, 1)
            | if_contains(flags, TextStyle::Underline, 2)
            | if_contains(flags, TextStyle::Blink, 4)
    }
}

#[repr(transparent)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct OutputSpan {
    inner: Output,
}

impl OutputSpan {
    pub const fn cast(output: &[Output]) -> &[Self] {
        // SAFETY: #[repr(transparent)]
        unsafe { &*(ptr::from_ref(output) as *const [Self]) }
    }

    pub const fn text_span(&self) -> *const TextSpan {
        let OutputFragment::Text(fragment) = &self.inner.fragment else {
            return ptr::null();
        };
        ptr::from_ref(fragment).cast()
    }
}
