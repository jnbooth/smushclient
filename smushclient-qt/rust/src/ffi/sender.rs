use std::mem;

use crate::sender::{AliasRust, TimerRust, TriggerRust};
use cxx::{type_id, ExternType};
use smushclient::AliasOutcome;

const fn flag_if(flag: ffi::AliasOutcome, pred: bool) -> u8 {
    if pred {
        flag.repr
    } else {
        0
    }
}

#[repr(transparent)]
pub struct AliasOutcomes(pub u8);
const _: [(); mem::size_of::<AliasOutcomes>()] = [(); mem::size_of::<ffi::AliasOutcome>()];

unsafe impl ExternType for AliasOutcomes {
    type Id = type_id!("AliasOutcomes");
    type Kind = cxx::kind::Trivial;
}

impl From<AliasOutcome> for AliasOutcomes {
    fn from(value: AliasOutcome) -> Self {
        Self(
            flag_if(ffi::AliasOutcome::Remember, value.remember)
                | flag_if(ffi::AliasOutcome::Send, value.send)
                | flag_if(ffi::AliasOutcome::Display, value.display),
        )
    }
}

#[cxx_qt::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qtime.h");
        type QTime = cxx_qt_lib::QTime;
    }

    unsafe extern "C++" {
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

    #[qenum(Timer)]
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
    }

    #[auto_cxx_name]
    unsafe extern "RustQt" {
        fn get_user_send_to(self: &Timer) -> UserSendTarget;
        fn set_user_send_to(self: Pin<&mut Timer>, send_to: UserSendTarget);
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for Timer {}
    impl
        cxx_qt::Constructor<(*const SmushClient, usize), NewArguments = (*const SmushClient, usize)>
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
    }

    #[auto_cxx_name]
    unsafe extern "RustQt" {
        fn get_user_send_to(self: &Alias) -> UserSendTarget;
        fn set_user_send_to(self: Pin<&mut Alias>, send_to: UserSendTarget);
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for Alias {}
    impl
        cxx_qt::Constructor<(*const SmushClient, usize), NewArguments = (*const SmushClient, usize)>
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
    }

    #[auto_cxx_name]
    unsafe extern "RustQt" {
        fn get_user_send_to(self: &Trigger) -> UserSendTarget;
        fn set_user_send_to(self: Pin<&mut Trigger>, send_to: UserSendTarget);
    }

    impl cxx_qt::Constructor<(), NewArguments = ()> for Trigger {}
    impl
        cxx_qt::Constructor<(*const SmushClient, usize), NewArguments = (*const SmushClient, usize)>
        for Trigger
    {
    }
}
