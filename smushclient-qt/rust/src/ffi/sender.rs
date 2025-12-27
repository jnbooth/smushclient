#![allow(clippy::elidable_lifetime_names)]
use cxx::type_id;
use cxx_qt_lib::{QFlag, QFlags};
use smushclient::AliasOutcome;

use crate::sender::{AliasRust, TimerRust, TriggerRust};

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qtime.h");
        type QTime = cxx_qt_lib::QTime;
    }

    extern "C++" {
        include!("forward.h");

        #[cxx_name = "SmushClientBase"]
        type SmushClient = crate::ffi::SmushClient;
    }

    enum AliasOutcome {
        Remember = 1,
        Send = 2,
        Display = 4,
    }

    #[repr(i32)]
    enum SendTarget {
        World,
        Command,
        Output,
        Status,
        NotepadNew,
        NotepadAppend,
        Log,
        NotepadReplace,
        WorldDelay,
        Variable,
        Execute,
        Speedwalk,
        Script,
        WorldImmediate,
        ScriptAfterOmit,
    }

    #[repr(i32)]
    enum UserSendTarget {
        World,
        Command,
        Output,
        Status,
        NotepadNew,
        NotepadAppend,
        NotepadReplace,
        Log,
        Variable,
        Script,
    }

    #[repr(i32)]
    enum Occurrence {
        Time,
        Interval,
    }

    #[auto_cxx_name]
    extern "RustQt" {
        // Sender
        #[qobject]
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Timer
        #[qproperty(Occurrence, occurrence)]
        #[qproperty(QTime, at_time)]
        #[qproperty(i32, every_hour)]
        #[qproperty(i32, every_minute)]
        #[qproperty(i32, every_second)]
        #[qproperty(u32, every_millisecond)]
        #[qproperty(bool, active_closed)]
        type Timer = super::TimerRust;

        fn get_user_send_to(self: &Timer) -> UserSendTarget;
        fn set_user_send_to(self: Pin<&mut Timer>, send_to: UserSendTarget);
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for Timer {}
    impl<'a> cxx_qt::Constructor<(&'a SmushClient, usize), NewArguments = (&'a SmushClient, usize)>
        for Timer
    {
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        // Sender
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Reaction
        #[qproperty(i32, sequence)]
        #[qproperty(QString, pattern)]
        #[qproperty(bool, ignore_case)]
        #[qproperty(bool, keep_evaluating)]
        #[qproperty(bool, is_regex)]
        #[qproperty(bool, expand_variables)]
        #[qproperty(bool, repeats)]
        // Alias
        #[qproperty(bool, echo_alias)]
        #[qproperty(bool, menu)]
        #[qproperty(bool, omit_from_command_history)]
        type Alias = super::AliasRust;

        fn get_user_send_to(self: &Alias) -> UserSendTarget;
        fn set_user_send_to(self: Pin<&mut Alias>, send_to: UserSendTarget);
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for Alias {}
    impl<'a> cxx_qt::Constructor<(&'a SmushClient, usize), NewArguments = (&'a SmushClient, usize)>
        for Alias
    {
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        // Sender
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Reaction
        #[qproperty(i32, sequence)]
        #[qproperty(QString, pattern)]
        #[qproperty(bool, ignore_case)]
        #[qproperty(bool, keep_evaluating)]
        #[qproperty(bool, is_regex)]
        #[qproperty(bool, expand_variables)]
        #[qproperty(bool, repeats)]
        // Trigger
        #[qproperty(bool, change_foreground)]
        #[qproperty(QColor, foreground_color)]
        #[qproperty(bool, change_background)]
        #[qproperty(QColor, background_color)]
        #[qproperty(bool, make_bold)]
        #[qproperty(bool, make_italic)]
        #[qproperty(bool, make_underline)]
        #[qproperty(QString, sound)]
        #[qproperty(bool, sound_if_inactive)]
        #[qproperty(bool, lowercase_wildcard)]
        #[qproperty(bool, multi_line)]
        #[qproperty(i32, lines_to_match)]
        type Trigger = super::TriggerRust;

        fn get_user_send_to(self: &Trigger) -> UserSendTarget;
        fn set_user_send_to(self: Pin<&mut Trigger>, send_to: UserSendTarget);
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for Trigger {}
    impl<'a> cxx_qt::Constructor<(&'a SmushClient, usize), NewArguments = (&'a SmushClient, usize)>
        for Trigger
    {
    }
}

pub type AliasOutcomes = QFlags<ffi::AliasOutcome>;

impl ffi::AliasOutcome {
    pub fn to_qflags(outcome: AliasOutcome) -> AliasOutcomes {
        let mut outcomes = AliasOutcomes::new();
        if outcome.remember {
            outcomes |= ffi::AliasOutcome::Remember;
        }
        if outcome.send {
            outcomes |= ffi::AliasOutcome::Send;
        }
        if outcome.display {
            outcomes |= ffi::AliasOutcome::Display;
        }
        outcomes
    }
}

// SAFETY: Static checks on C++ side.
unsafe impl QFlag for ffi::AliasOutcome {
    type TypeId = type_id!("AliasOutcomes");

    type Repr = u8;

    fn to_repr(self) -> Self::Repr {
        self.repr
    }
}
