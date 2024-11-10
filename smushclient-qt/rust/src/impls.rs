use std::mem;

use super::ffi;
use crate::bridge::TextStyles;
use crate::convert::Convert;
use crate::sender::OutputSpan;
use cxx_qt_lib::QString;
use enumeration::EnumSet;
use mud_transformer::mxp::{Link, SendTo};
use mud_transformer::{TelnetSource, TelnetVerb, TextStyle, UseMxp};
use smushclient::world::{AutoConnect, LogFormat, LogMode, ScriptRecompile};
use smushclient::{
    AliasBool, CommandSource, SendRequest, SendScriptRequest, TimerBool, TimerConstructible,
    TriggerBool,
};
use smushclient_plugins::{PluginIndex, Regex, SendTarget, Timer};

impl_convert_enum!(ffi::SendTo, SendTo, Internet, World, Input);

impl_convert_enum!(ffi::TelnetSource, TelnetSource, Client, Server);

impl_convert_enum!(ffi::TelnetVerb, TelnetVerb, Do, Dont, Will, Wont);

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

impl_convert_enum!(ffi::CommandSource, CommandSource, Hotkey, Link, User);

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

impl<'a> From<&'a Link> for ffi::Link<'a> {
    fn from(value: &'a Link) -> Self {
        Self {
            action: QString::from(&value.action),
            hint: value.hint.convert(),
            prompts: QString::from(&value.prompts.join("|")),
            sendto: value.sendto.into(),
            expires: match &value.expires {
                Some(expires) => expires.as_str(),
                None => "",
            },
        }
    }
}

impl<'a> From<SendRequest<'a>> for ffi::SendRequest {
    fn from(value: SendRequest<'a>) -> Self {
        Self {
            plugin: value.plugin,
            send_to: value.send_to.into(),
            text: QString::from(value.text),
            destination: QString::from(value.destination),
        }
    }
}

impl<'a> From<SendScriptRequest<'a>> for ffi::SendScriptRequest<'a> {
    fn from(value: SendScriptRequest<'a>) -> Self {
        let (wildcards, named_wildcards) = match value.wildcards {
            Some(captures) => {
                let mut wildcards = Vec::new();
                for i in 1..captures.len() {
                    match captures.get(i) {
                        Some(captures) => wildcards.push(Regex::expect(captures.as_bytes())),
                        None => wildcards.push(""),
                    }
                }
                let named_wildcards = value
                    .regex
                    .capture_names()
                    .iter()
                    .filter_map(|name| {
                        let name = name.as_ref()?;
                        let value = captures.name(name)?;
                        Some(ffi::NamedWildcard {
                            name,
                            value: Regex::expect(value.as_bytes()),
                        })
                    })
                    .collect();
                (wildcards, named_wildcards)
            }
            None => (Vec::new(), Vec::new()),
        };
        Self {
            plugin: value.plugin,
            script: value.script,
            label: value.label,
            line: value.line,
            wildcards,
            named_wildcards,
            output: OutputSpan::cast(value.output),
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
            destination: QString::from(timer.destination()),
        }
    }
}

const _: [(); mem::size_of::<EnumSet<TextStyle>>()] = [(); mem::size_of::<TextStyles>()];

impl From<EnumSet<TextStyle>> for TextStyles {
    fn from(value: EnumSet<TextStyle>) -> Self {
        Self(value.to_raw())
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
