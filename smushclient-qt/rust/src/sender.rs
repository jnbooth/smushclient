#![allow(
    clippy::cast_possible_truncation,
    clippy::cast_possible_wrap,
    clippy::cast_sign_loss
)]
use std::pin::Pin;
use std::ptr;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use cxx_qt::{CxxQtType, Initialize};
use cxx_qt_lib::{QColor, QString, QTime};
use flagset::{FlagSet, Flags};
use mud_transformer::opt::mxp::RgbColor;
use mud_transformer::output::{Output, OutputFragment, TextFragment, TextStyle};
use smushclient_plugins::hmsn::Hmsn;
use smushclient_plugins::newline::ensure_crlf;
use smushclient_plugins::{Alias, Occurrence, Reaction, RegexError, Sender, Timer, Trigger};

use crate::convert::{Convert, impl_deref};
use crate::ffi;

const MILLISECONDS_PER_SECOND: u32 = 1000;
const NANOSECONDS_PER_MILLISECOND: u32 = 1_000_000;
const SECONDS_PER_MINUTE: i32 = 60;
const MINUTES_PER_HOUR: i32 = 60;

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
        let text = char::decode_utf16(ensure_crlf(value.text.as_slice().iter().copied()))
            .map(|r| r.unwrap_or(char::REPLACEMENT_CHARACTER))
            .collect();
        Self {
            send_to: value.send_to.try_into().unwrap_or_default(),
            label: String::from(&value.label),
            script: String::from(&value.script),
            group: String::from(&value.group),
            variable: String::from(&value.variable),
            text,
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
    pub every_millisecond: i32,
    pub active_closed: bool,
    pub id: u16,
}

impl_deref!(TimerRust, SenderRust, send);

impl From<&Timer> for TimerRust {
    fn from(timer: &Timer) -> Self {
        let send = SenderRust::from(&timer.send);

        match timer.occurrence {
            Occurrence::Time(time) => {
                let seconds = time.num_seconds_from_midnight();
                let msecs = seconds * MILLISECONDS_PER_SECOND;
                Self {
                    send,
                    occurrence: ffi::Occurrence::Time,
                    at_time: QTime::from_msecs_since_start_of_day(msecs as i32),
                    every_millisecond: 0,
                    every_second: 0,
                    every_minute: 0,
                    every_hour: 0,
                    active_closed: timer.active_closed,
                    id: timer.id,
                }
            }
            Occurrence::Interval(duration) => {
                let seconds = duration.as_secs() as i32;
                let minutes = seconds / SECONDS_PER_MINUTE;
                Self {
                    send,
                    occurrence: ffi::Occurrence::Interval,
                    at_time: QTime::default(),
                    every_millisecond: duration.subsec_millis() as i32,
                    every_second: (seconds % SECONDS_PER_MINUTE),
                    every_minute: (minutes % MINUTES_PER_HOUR),
                    every_hour: (minutes / MINUTES_PER_HOUR),
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
                let msecs = value.at_time.msecs_since_start_of_day() as u32;
                let seconds = msecs / MILLISECONDS_PER_SECOND;
                let nanos = msecs % MILLISECONDS_PER_SECOND * NANOSECONDS_PER_MILLISECOND;
                NaiveTime::from_num_seconds_from_midnight_opt(seconds, nanos)
                    .unwrap_or_default()
                    .into()
            }
            ffi::Occurrence::Interval => Duration::from(Hmsn(
                value.every_hour as u64,
                value.every_minute as u64,
                value.every_second as u64,
                (value.every_millisecond as u32) * NANOSECONDS_PER_MILLISECOND,
            ))
            .into(),
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
            clipboard_arg: trigger.clipboard_arg.into(),
        }
    }
}

impl TryFrom<&TriggerRust> for Trigger {
    type Error = RegexError;

    fn try_from(value: &TriggerRust) -> Result<Self, Self::Error> {
        Ok(Self {
            reaction: Reaction::try_from(&value.reaction)?,
            change_foreground: value.change_foreground,
            foreground_color: value.foreground_color.convert().unwrap_or(RgbColor::WHITE),
            change_background: value.change_background,
            background_color: value.background_color.convert().unwrap_or(RgbColor::BLACK),
            make_bold: value.make_bold,
            make_italic: value.make_italic,
            make_underline: value.make_underline,
            sound: String::from(&value.sound),
            sound_if_inactive: value.sound_if_inactive,
            lowercase_wildcard: value.lowercase_wildcard,
            multi_line: value.multi_line,
            lines_to_match: value.lines_to_match.try_into().unwrap_or(u8::MAX),
            clipboard_arg: value.clipboard_arg.try_into().unwrap_or_default(),
        })
    }
}

macro_rules! impl_sender_constructors {
    ($t:ident) => {
        impl Initialize for ffi::$t {
            fn initialize(self: Pin<&mut Self>) {}
        }

        impl<'a> cxx_qt::Constructor<(&'a ffi::SmushClient, usize)> for ffi::$t {
            type BaseArguments = ();
            type InitializeArguments = ();
            type NewArguments = (&'a ffi::SmushClient, usize);
            fn route_arguments(
                args: Self::NewArguments,
            ) -> (
                Self::NewArguments,
                Self::BaseArguments,
                Self::InitializeArguments,
            ) {
                (args, (), ())
            }
            fn new((client, index): (&'a ffi::SmushClient, usize)) -> Self::Rust {
                let sender = client
                    .rust()
                    .client
                    .world_plugin()
                    .senders::<$t>()
                    .get(index);
                match sender {
                    Some(sender) => Self::Rust::from(&*sender),
                    None => Self::Rust::default(),
                }
            }
        }
    };
}

impl_sender_constructors!(Alias);
impl_sender_constructors!(Timer);
impl_sender_constructors!(Trigger);

#[inline(always)]
fn if_contains<E: Flags>(set: FlagSet<E>, value: E, flag: u8) -> u8 {
    if set.contains(value) { flag } else { 0 }
}

#[repr(transparent)]
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct TextSpan {
    inner: TextFragment,
}

fn color_code(color: Option<RgbColor>) -> i32 {
    match color {
        Some(color) => color.code() as i32,
        None => -1,
    }
}

impl TextSpan {
    pub fn foreground(&self) -> i32 {
        color_code(self.inner.foreground)
    }

    pub fn background(&self) -> i32 {
        color_code(self.inner.background)
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
