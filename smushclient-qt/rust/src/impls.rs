use super::ffi;
use crate::convert::Convert;
use cxx_qt_lib::QString;
use mud_transformer::mxp::{Link, SendTo};
use mud_transformer::{TextStyle, UseMxp};
use smushclient::world::{AutoConnect, LogFormat, LogMode, ProxyType, ScriptRecompile};
use smushclient::{AliasBool, AliasOutcome, TimerBool, TimerConstructible, TriggerBool};
use smushclient_plugins::{PluginIndex, SendTarget, Timer};

impl_convert_enum!(ffi::SendTo, SendTo, Internet, World, Input);

impl_convert_enum_opt!(ffi::ProxyType, ProxyType, None, Socks4, Socks5);

impl_convert_enum_opt!(ffi::AutoConnect, AutoConnect, None, Mush, Diku, Mxp);

impl_convert_enum!(
    ffi::ScriptRecompile,
    ScriptRecompile,
    Confirm,
    Always,
    Never
);

impl_convert_enum!(ffi::LogFormat, LogFormat, Text, Html, Raw);

impl_convert_enum!(ffi::LogMode, LogMode, Append, Overwrite);

impl_convert_enum!(ffi::UseMxp, UseMxp, Command, Query, Always, Never);

impl_convert_enum!(
    ffi::SendTarget,
    SendTarget,
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
);

impl_convert_enum!(
    ffi::AliasBool,
    AliasBool,
    Enabled,
    OneShot,
    Temporary,
    OmitFromOutput,
    OmitFromLog,
    IgnoreCase,
    KeepEvaluating,
    IsRegex,
    ExpandVariables,
    Repeats,
    EchoAlias,
    Menu,
    OmitFromCommandHistory,
);

impl_convert_enum!(
    ffi::TimerBool,
    TimerBool,
    Enabled,
    OneShot,
    Temporary,
    OmitFromOutput,
    OmitFromLog,
    ActiveClosed,
);

impl_convert_enum!(
    ffi::TriggerBool,
    TriggerBool,
    Enabled,
    OneShot,
    Temporary,
    OmitFromOutput,
    OmitFromLog,
    IgnoreCase,
    KeepEvaluating,
    IsRegex,
    ExpandVariables,
    Repeats,
    ChangeForeground,
    ChangeBackground,
    MakeBold,
    MakeItalic,
    MakeUnderline,
    SoundIfInactive,
    LowercaseWildcard,
    MultiLine
);

impl Default for ffi::SendTarget {
    fn default() -> Self {
        Self::World
    }
}

impl Default for ffi::Occurrence {
    fn default() -> Self {
        Self::Interval
    }
}

impl Default for ffi::ScriptRecompile {
    fn default() -> Self {
        Self::Confirm
    }
}

impl Default for ffi::LogFormat {
    fn default() -> Self {
        Self::Text
    }
}

impl Default for ffi::LogMode {
    fn default() -> Self {
        Self::Append
    }
}

impl Default for ffi::UseMxp {
    fn default() -> Self {
        Self::Command
    }
}

impl From<&Link> for ffi::Link {
    fn from(value: &Link) -> Self {
        Self {
            action: QString::from(&value.action),
            hint: value.hint.convert(),
            prompts: QString::from(&value.prompts.join("|")),
            sendto: value.sendto.into(),
        }
    }
}

impl TimerConstructible for ffi::SendTimer {
    fn construct(plugin: PluginIndex, timer: &Timer) -> Self {
        Self {
            active_closed: timer.active_closed,
            label: timer.label.clone(),
            plugin,
            script: timer.script.clone(),
            target: timer.send_to.into(),
            text: QString::from(&timer.text),
        }
    }
}

macro_rules! assert_textstyle {
    ($i:ident) => {
        const _: [(); TextStyle::$i.bit() as usize] = [(); ffi::TextStyle::$i.repr as usize];
    };
}

assert_textstyle!(Blink);
assert_textstyle!(Bold);
assert_textstyle!(Highlight);
assert_textstyle!(NonProportional);
assert_textstyle!(Small);
assert_textstyle!(Strikeout);
assert_textstyle!(Underline);
assert_textstyle!(Inverse);

const fn flag_if(flag: ffi::AliasOutcome, pred: bool) -> u8 {
    if pred {
        flag.repr
    } else {
        0
    }
}

pub const fn convert_alias_outcome(outcome: AliasOutcome) -> u8 {
    flag_if(ffi::AliasOutcome::Display, outcome.display)
        | flag_if(ffi::AliasOutcome::Remember, outcome.remember)
        | flag_if(ffi::AliasOutcome::Send, outcome.send)
}
