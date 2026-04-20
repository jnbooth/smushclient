use cxx_qt_lib::QString;
use mud_transformer::term::DynamicColor;
use mud_transformer::{TelnetSource, TelnetVerb, UseMxp};
use smushclient::world::{AutoConnect, LogFormat, LogMode, MxpDebugLevel, ScriptRecompile};
use smushclient::{
    AudioSinkStatus, CommandSource, SendRequest, SendScriptRequest, TimerConstructible,
};
use smushclient_plugins::{Plugin, PluginIndex, SendTarget, Timer};

use super::ffi;
use crate::convert::impl_convert_enum;
use crate::sender::OutputSpan;

impl_convert_enum!(ffi::TelnetSource, TelnetSource, Client, Server);

impl_convert_enum!(ffi::TelnetVerb, TelnetVerb, Do, Dont, Will, Wont);

impl_convert_enum!(
    ffi::DynamicColor,
    DynamicColor,
    TextForeground,
    TextBackground,
    TextCursor,
    MouseForeground,
    MouseBackground,
    TektronixForeground,
    TektronixBackground,
    Highlight,
    TektronixCursor,
);

impl_convert_enum!(ffi::AutoConnect, AutoConnect, None, Mush, Diku, Mxp);

impl_convert_enum!(
    ffi::ScriptRecompile,
    ScriptRecompile,
    Confirm,
    Always,
    Never,
);

impl_convert_enum!(ffi::LogFormat, LogFormat, Text, Html, Raw);

impl_convert_enum!(ffi::LogMode, LogMode, Append, Overwrite);

impl_convert_enum!(
    ffi::MXPDebugLevel,
    MxpDebugLevel,
    None,
    Error,
    Warning,
    Info,
    All,
);

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
    ffi::CommandSource,
    CommandSource,
    User,
    Hotkey,
    Link,
    Execute
);

impl_convert_enum!(
    ffi::AudioSinkStatus,
    AudioSinkStatus,
    OutOfRange,
    Done,
    Playing,
    Looping
);

impl<'a> From<SendRequest<'a>> for ffi::SendRequest {
    fn from(value: SendRequest<'a>) -> Self {
        Self {
            plugin: value.plugin,
            send_to: value.send_to.into(),
            echo: value.echo,
            log: value.log,
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
                for capture in &captures {
                    wildcards.push(capture);
                }
                let named_wildcards = value
                    .regex
                    .capture_names()
                    .iter()
                    .filter_map(|name| {
                        let name = name.as_ref()?;
                        Some(ffi::NamedWildcard {
                            name,
                            value: captures.name(name)?.as_str(),
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
            script: timer.script.clone(),
            request: ffi::SendRequest {
                plugin,
                send_to: timer.send_to.into(),
                echo: !timer.omit_from_output,
                log: !timer.omit_from_log,
                text: QString::from(&timer.text),
                destination: QString::from(timer.destination()),
            },
        }
    }
}

impl From<&Plugin> for ffi::PluginPack {
    fn from(value: &Plugin) -> Self {
        Self {
            id: value.metadata.id.clone(),
            name: value.metadata.name.clone(),
            path: QString::from(&*value.metadata.path.to_string_lossy()),
            script_data: value.script.as_ptr(),
            script_size: value.script.len(),
        }
    }
}

impl ffi::SenderKind {
    pub const fn not_found(self) -> ffi::ApiCode {
        match self {
            Self::Alias => ffi::ApiCode::AliasNotFound,
            Self::Timer => ffi::ApiCode::TimerNotFound,
            Self::Trigger => ffi::ApiCode::TriggerNotFound,
            _ => ffi::ApiCode::BadParameter,
        }
    }
}
