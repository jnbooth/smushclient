use std::num::NonZero;
use std::path::PathBuf;
use std::sync::Arc;
use std::time::Duration;

use chrono::{NaiveTime, Timelike};
use mud_transformer::mxp::{AudioRepetition, Heading, RgbColor, SendTo};
use mud_transformer::{
    ControlFragment, EntityFragment, MxpFragment, OutputFragment, TelnetFragment, TelnetSource,
    TelnetVerb, UseMxp,
};
use smushclient::world::{
    AutoConnect, LogFormat, LogMode, MxpDebugLevel, Numpad, NumpadMapping, ScriptRecompile,
};
use smushclient::{SendRequest, SendScriptRequest, World};
use smushclient_plugins::{
    Alias, CursorVec, Occurrence, Reaction, SendTarget, Sender, Timer, Trigger,
};

use crate::convert::{Convert, impl_convert, impl_convert_enum, impl_convert_struct};
use crate::error::UnsupportedError;
use crate::ffi;

impl Convert<PathBuf> for String {
    fn from_ffi(value: Self) -> PathBuf {
        PathBuf::from(value)
    }

    fn to_ffi(value: PathBuf) -> Self {
        value.to_string_lossy().into_owned()
    }
}

impl<T, Ffi: Convert<T>> Convert<CursorVec<T>> for Vec<Ffi> {
    fn from_ffi(value: Self) -> CursorVec<T> {
        value.into_iter().map(Convert::from_ffi).collect()
    }

    fn to_ffi(value: CursorVec<T>) -> Self {
        Vec::from(value).into_iter().map(Convert::to_ffi).collect()
    }
}

impl_convert_enum!(ffi::AutoConnect, AutoConnect, None, Mush, Diku, Mxp);

impl_convert_enum!(
    ffi::ScriptRecompile,
    ScriptRecompile,
    Confirm,
    Always,
    Never,
);

impl_convert_enum!(ffi::UseMxp, UseMxp, Command, Query, Always, Never);

impl_convert_enum!(ffi::LogFormat, LogFormat, Text, Html, Raw);

impl_convert_enum!(ffi::LogMode, LogMode, Append, Overwrite);

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

impl_convert!(ffi::ColorOption, Option<RgbColor>);

impl_convert_enum!(
    ffi::MXPDebugLevel,
    MxpDebugLevel,
    None,
    Error,
    Warning,
    Info,
    All,
);

impl_convert_enum!(ffi::SendTo, SendTo, World, Input, Internet);

impl_convert_enum!(ffi::TelnetSource, TelnetSource, Client, Server);

impl_convert_enum!(ffi::TelnetVerb, TelnetVerb, Do, Dont, Will, Wont);

impl_convert_struct!(
    ffi::Sender,
    Sender,
    send_to,
    label,
    script,
    group,
    variable,
    text,
    enabled,
    one_shot,
    temporary,
    omit_from_output,
    omit_from_log,
    id,
    userdata,
);

impl From<Reaction> for ffi::Reaction {
    fn from(value: Reaction) -> Self {
        Self {
            sequence: value.sequence,
            pattern: value.pattern,
            send: value.send.into(),
            ignore_case: value.ignore_case,
            keep_evaluating: value.keep_evaluating,
            is_regex: value.is_regex,
            expand_variables: value.expand_variables,
            repeats: value.repeats,
        }
    }
}
impl From<ffi::Reaction> for Reaction {
    fn from(value: ffi::Reaction) -> Self {
        Self {
            regex: Arc::new(
                Reaction::make_regex(&value.pattern, value.is_regex, value.ignore_case)
                    .expect("invalid regular expression"),
            ),
            sequence: value.sequence,
            pattern: value.pattern,
            send: value.send.into(),
            ignore_case: value.ignore_case,
            keep_evaluating: value.keep_evaluating,
            is_regex: value.is_regex,
            expand_variables: value.expand_variables,
            repeats: value.repeats,
        }
    }
}
impl Convert<Reaction> for ffi::Reaction {
    fn from_ffi(value: Self) -> Reaction {
        <Reaction>::from(value)
    }
    fn to_ffi(value: Reaction) -> Self {
        Self::from(value)
    }
}

impl_convert_struct!(
    ffi::Alias,
    Alias,
    reaction,
    echo_alias,
    menu,
    omit_from_command_history,
);

impl_convert_struct!(
    ffi::Trigger,
    Trigger,
    reaction,
    change_foreground,
    foreground_color,
    change_background,
    background_color,
    make_bold,
    make_italic,
    make_underline,
    sound,
    sound_if_inactive,
    lowercase_wildcard,
    multi_line,
    lines_to_match,
    clipboard_arg,
);

impl_convert!(ffi::Occurrence, Occurrence);

impl_convert_struct!(ffi::Timer, Timer, send, occurrence, active_closed);

impl_convert_struct!(ffi::RgbColor, RgbColor, r, g, b);

impl_convert_struct!(
    ffi::Numpad,
    Numpad,
    key_0,
    key_1,
    key_2,
    key_3,
    key_4,
    key_5,
    key_6,
    key_7,
    key_8,
    key_9,
    key_period,
    key_slash,
    key_asterisk,
    key_minus,
    key_plus,
);

impl_convert_struct!(ffi::NumpadMapping, NumpadMapping, base, modified);

impl_convert_struct!(
    ffi::World,
    World,
    // Connecting
    name,
    site,
    port,
    use_ssl,
    use_proxy,
    proxy_server,
    proxy_port,
    proxy_username,
    proxy_password,
    save_world_automatically,
    // Login
    player,
    password,
    connect_method,
    connect_text,
    // Logging
    log_file_preamble,
    log_file_postamble,
    log_format,
    log_in_colour,
    log_output,
    log_input,
    log_notes,
    log_mode,
    auto_log_file_name,
    write_world_name_to_log,
    log_line_preamble_output,
    log_line_preamble_input,
    log_line_preamble_notes,
    log_line_postamble_output,
    log_line_postamble_input,
    log_line_postamble_notes,
    log_script_errors,
    // Timers
    timers,
    enable_timers,
    // Output
    show_bold,
    show_italic,
    show_underline,
    indent_paras,
    ansi_colours,
    use_default_colours,
    display_my_input,
    echo_colour,
    echo_background_colour,
    keep_commands_on_same_line,
    new_activity_sound,
    line_information,
    //MUD
    use_mxp,
    ignore_mxp_colour_changes,
    use_custom_link_colour,
    hyperlink_colour,
    mud_can_change_link_colour,
    underline_hyperlinks,
    mud_can_remove_underline,
    hyperlink_adds_to_command_history,
    echo_hyperlink_in_output_window,
    terminal_identification,
    disable_compression,
    naws,
    carriage_return_clears_line,
    utf_8,
    convert_ga_to_newline,
    no_echo_off,
    enable_command_stack,
    command_stack_character,
    mxp_debug_level,
    // Triggers
    triggers,
    enable_triggers,
    enable_trigger_sounds,
    // Aliases
    aliases,
    enable_aliases,
    // Numpad
    numpad_shortcuts,
    keypad_enable,
    hotkey_adds_to_command_history,
    echo_hotkey_in_output_window,
    // Scripting
    enable_scripts,
    world_script,
    script_reload_option,
    note_text_colour,
    note_background_colour,
    script_errors_to_output_window,
    error_text_colour,
    error_background_colour,
    // Hidden
    plugins,
);

impl<'a> From<SendRequest<'a>> for ffi::SendRequest {
    fn from(value: SendRequest<'a>) -> Self {
        Self {
            plugin: value.plugin,
            send_to: value.send_to.into(),
            text: value.text.to_owned(),
            destination: value.destination.to_owned(),
        }
    }
}

impl<'a> From<SendScriptRequest<'a>> for ffi::SendScriptRequest {
    fn from(value: SendScriptRequest<'a>) -> Self {
        let wildcards = match value.wildcards {
            Some(captures) => captures.iter().map(ToOwned::to_owned).collect(),
            None => Vec::new(),
        };
        Self {
            plugin: value.plugin,
            script: value.script.to_owned(),
            label: value.label.to_owned(),
            line: value.line.to_owned(),
            wildcards,
        }
    }
}

impl From<ffi::ColorOption> for Option<RgbColor> {
    fn from(value: ffi::ColorOption) -> Self {
        match value {
            ffi::ColorOption::Some(color) => Some(color.into()),
            ffi::ColorOption::None => None,
        }
    }
}

impl From<Option<RgbColor>> for ffi::ColorOption {
    fn from(value: Option<RgbColor>) -> Self {
        match value {
            Some(color) => ffi::ColorOption::Some(color.into()),
            None => ffi::ColorOption::None,
        }
    }
}

impl From<Occurrence> for ffi::Occurrence {
    fn from(value: Occurrence) -> Self {
        match value {
            Occurrence::Time(time) => Self::Time {
                h: time.hour(),
                m: time.minute(),
                s: time.second(),
            },
            Occurrence::Interval(interval) => Self::Interval {
                s: interval.as_secs(),
            },
        }
    }
}

impl From<ffi::Occurrence> for Occurrence {
    fn from(value: ffi::Occurrence) -> Self {
        match value {
            ffi::Occurrence::Interval { s } => Occurrence::Interval(Duration::from_secs(s)),
            ffi::Occurrence::Time { h, m, s } => {
                Occurrence::Time(NaiveTime::from_hms_opt(h, m, s).unwrap_or_default())
            }
        }
    }
}

impl Convert<AudioRepetition> for u32 {
    fn from_ffi(value: Self) -> AudioRepetition {
        match NonZero::new(value) {
            Some(count) => AudioRepetition::Count(count),
            None => AudioRepetition::Forever,
        }
    }

    fn to_ffi(value: AudioRepetition) -> Self {
        match value {
            AudioRepetition::Forever => 0,
            AudioRepetition::Count(count) => count.get(),
        }
    }
}

impl TryFrom<ControlFragment> for ffi::ControlFragment {
    type Error = UnsupportedError;

    fn try_from(value: ControlFragment) -> Result<Self, Self::Error> {
        match value {
            ControlFragment::Beep => Ok(Self::Beep),
            ControlFragment::CarriageReturn => Ok(Self::CarriageReturn),
            _ => Err(UnsupportedError("control fragment")),
        }
    }
}

impl From<TelnetFragment> for ffi::TelnetFragment {
    fn from(value: TelnetFragment) -> Self {
        match value {
            TelnetFragment::Msdp { name, value } => Self::Msdp {
                name: String::from_utf8_lossy(&name).into_owned(),
                value: match value {
                    mud_transformer::MsdpValue::String(s) => {
                        String::from_utf8_lossy(&s).into_owned()
                    }
                    _ => String::new(),
                },
            },
            TelnetFragment::GoAhead => Self::GoAhead,
            TelnetFragment::Mxp { enabled } => Self::Mxp { enabled },
            TelnetFragment::Naws => Self::Naws,
            TelnetFragment::Negotiation { source, verb, code } => Self::Negotiation {
                source: source.into(),
                verb: verb.into(),
                code,
            },
            TelnetFragment::SetEcho { should_echo } => Self::SetEcho { should_echo },
            TelnetFragment::ServerStatus { variable, value } => Self::ServerStatus {
                variable: variable.into(),
                value: value.into(),
            },
            TelnetFragment::Subnegotiation { code, data } => Self::Subnegotiation {
                code,
                data: data.to_vec(),
            },
        }
    }
}

impl Clone for ffi::TelnetFragment {
    fn clone(&self) -> Self {
        match self {
            Self::Msdp { name, value } => Self::Msdp {
                name: name.clone(),
                value: value.clone(),
            },
            Self::GoAhead => Self::GoAhead,
            Self::Mxp { enabled } => Self::Mxp { enabled: *enabled },
            Self::Naws => Self::Naws,
            Self::Negotiation { source, verb, code } => Self::Negotiation {
                source: *source,
                verb: *verb,
                code: *code,
            },
            Self::ServerStatus { variable, value } => Self::ServerStatus {
                variable: variable.clone(),
                value: value.clone(),
            },
            Self::SetEcho { should_echo } => Self::SetEcho {
                should_echo: *should_echo,
            },
            Self::Subnegotiation { code, data } => Self::Subnegotiation {
                code: *code,
                data: data.clone(),
            },
        }
    }
}

impl_convert_enum!(ffi::Heading, Heading, H1, H2, H3, H4, H5, H6);

impl From<EntityFragment> for ffi::EntityFragment {
    fn from(value: EntityFragment) -> Self {
        match value {
            EntityFragment::Set {
                name,
                value,
                publish,
                is_variable,
            } => Self::Set {
                name,
                value,
                publish,
                is_variable,
            },
            EntityFragment::Unset { name, is_variable } => Self::Unset { name, is_variable },
        }
    }
}

impl TryFrom<MxpFragment> for ffi::MxpFragment {
    type Error = UnsupportedError;

    fn try_from(value: MxpFragment) -> Result<Self, UnsupportedError> {
        match value {
            MxpFragment::Entity(fragment) => Ok(Self::Entity(fragment.into())),
            MxpFragment::Error(error) => Ok(Self::Error(error.to_string())),
            MxpFragment::ExpireLinks(_) => Err(UnsupportedError("<EXPIRE>")),
            MxpFragment::FileFilter(_) => Err(UnsupportedError("<FILTER>")),
            MxpFragment::Gauge(_) => Err(UnsupportedError("<GAUGE>")),
            MxpFragment::Music(_) | MxpFragment::MusicOff => Err(UnsupportedError("<MUSIC>")),
            MxpFragment::Relocate(_) => Err(UnsupportedError("<RELOCATE>")),
            MxpFragment::Sound(_) | MxpFragment::SoundOff => Err(UnsupportedError("<SOUND>")),
            MxpFragment::StatusBar(_) => Err(UnsupportedError("<STAT>")),
        }
    }
}

impl TryFrom<OutputFragment> for ffi::OutputFragment {
    type Error = UnsupportedError;

    fn try_from(value: OutputFragment) -> Result<Self, Self::Error> {
        Ok(match value {
            OutputFragment::Control(fragment) => Self::Control(fragment.try_into()?),
            OutputFragment::Frame(_) => return Err(UnsupportedError("<frame>")),
            OutputFragment::Hr => Self::Hr,
            OutputFragment::Image(_) => return Err(UnsupportedError("<image>")),
            OutputFragment::LineBreak => Self::LineBreak,
            OutputFragment::Mxp(fragment) => Self::Mxp(fragment.try_into()?),
            OutputFragment::PageBreak => Self::PageBreak,
            OutputFragment::Telnet(telnet) => Self::Telnet(telnet.into()),
            OutputFragment::Text(text) => Self::Text(text.into()),
        })
    }
}
