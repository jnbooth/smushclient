use std::collections::HashMap;
use std::num::NonZeroU32;
use std::path::PathBuf;
use std::time::Duration;

use crate::convert::Convert;
use crate::error::UnsupportedError;
use crate::ffi;
use chrono::{NaiveTime, Timelike};
use mud_transformer::mxp::{AudioRepetition, Heading, RgbColor, SendTo};
use mud_transformer::{EffectFragment, EntityFragment, OutputFragment, TelnetFragment, UseMxp};
use smushclient::world::{AutoConnect, ColorPair, LogFormat, LogMode, ProxyType, ScriptRecompile};
use smushclient::{SendRequest, World};
use smushclient_plugins::{Alias, Occurrence, Reaction, Regex, SendTarget, Sender, Timer, Trigger};

impl Convert<PathBuf> for String {
    fn from_ffi(value: Self) -> PathBuf {
        PathBuf::from(value)
    }

    fn to_ffi(value: PathBuf) -> Self {
        value.to_string_lossy().into_owned()
    }
}

impl_convert_enum_opt!(ffi::ProxyType, ProxyType, Socks4, Socks5);

impl_convert_enum_opt!(ffi::AutoConnect, AutoConnect, Mush, Diku, Mxp);

impl_convert_enum!(
    ffi::ScriptRecompile,
    ScriptRecompile,
    Confirm,
    Always,
    Never
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

impl_convert_struct!(ffi::ColorPair, ColorPair, foreground, background,);

impl_convert_enum!(ffi::SendTo, SendTo, World, Input, Internet);

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
    omit_from_log
);

impl_convert_struct!(
    ffi::Reaction,
    Reaction,
    sequence,
    pattern,
    send,
    ignore_case,
    keep_evaluating,
    is_regex,
    expand_variables,
    repeats,
    regex
);

impl_convert_struct!(
    ffi::Alias,
    Alias,
    reaction,
    echo_alias,
    menu,
    omit_from_command_history
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
    lines_to_match
);

impl_convert!(ffi::Occurrence, Occurrence);

impl_convert_struct!(ffi::Timer, Timer, send, occurrence, active_closed);

impl_convert_struct!(ffi::RgbColor, RgbColor, r, g, b);

impl_convert_struct!(
    ffi::World,
    World,
    name,
    site,
    port,
    proxy_type,
    proxy_server,
    proxy_port,
    proxy_username,
    proxy_password,
    proxy_password_base64,
    save_world_automatically,
    player,
    password,
    connect_method,
    connect_text,
    log_file_preamble,
    log_file_postamble,
    log_format,
    log_output,
    log_input,
    log_notes,
    log_mode,
    auto_log_file_name,
    log_preamble_output,
    log_preamble_input,
    log_preamble_notes,
    log_postamble_output,
    log_postamble_input,
    log_postamble_notes,
    timers,
    enable_timers,
    treeview_timers,
    chat_name,
    auto_allow_snooping,
    accept_chat_connections,
    chat_port,
    validate_incoming_chat_calls,
    chat_colors,
    ignore_chat_colours,
    chat_message_prefix,
    chat_max_lines_per_message,
    chat_max_bytes_per_message,
    auto_allow_files,
    chat_file_save_directory,
    notes,
    beep_sound,
    pixel_offset,
    line_spacing,
    output_font,
    output_font_height,
    use_default_output_font,
    show_bold,
    show_italic,
    show_underline,
    new_activity_sound,
    max_output_lines,
    wrap_column,
    line_information,
    start_paused,
    auto_pause,
    unpause_on_send,
    flash_taskbar_icon,
    disable_compression,
    indent_paras,
    naws,
    carriage_return_clears_line,
    utf_8,
    auto_wrap_window_width,
    show_connect_disconnect,
    copy_selection_to_clipboard,
    auto_copy_to_clipboard_in_html,
    convert_ga_to_newline,
    terminal_identification,
    use_mxp,
    hyperlink_colour,
    use_custom_link_colour,
    mud_can_change_link_colour,
    underline_hyperlinks,
    mud_can_remove_underline,
    hyperlink_adds_to_command_history,
    echo_hyperlink_in_output_window,
    ignore_mxp_colour_changes,
    send_mxp_afk_response,
    use_default_colours,
    ansi_colors,
    custom_color,
    error_colour,
    triggers,
    enable_triggers,
    enable_trigger_sounds,
    treeview_triggers,
    display_my_input,
    echo_colors,
    enable_speed_walk,
    speed_walk_prefix,
    speed_walk_filler,
    speed_walk_delay,
    enable_command_stack,
    command_stack_character,
    input_colors,
    input_font,
    input_font_height,
    use_default_input_font,
    enable_spam_prevention,
    spam_line_count,
    spam_message,
    auto_repeat,
    lower_case_tab_completion,
    translate_german,
    translate_backslash_sequences,
    keep_commands_on_same_line,
    no_echo_off,
    tab_completion_lines,
    tab_completion_space,
    double_click_inserts,
    double_click_sends,
    escape_deletes_input,
    save_deleted_command,
    confirm_before_replacing_typing,
    arrow_keys_wrap,
    arrows_change_history,
    arrow_recalls_partial,
    alt_arrow_recalls_partial,
    ctrl_z_goes_to_end_of_buffer,
    ctrl_p_goes_to_previous_command,
    ctrl_n_goes_to_next_command,
    history_lines,
    aliases,
    enable_aliases,
    treeview_aliases,
    keypad_enable,
    keypad_shortcuts,
    enable_auto_say,
    autosay_exclude_non_alpha,
    autosay_exclude_macros,
    auto_say_override_prefix,
    auto_say_string,
    re_evaluate_auto_say,
    paste_line_preamble,
    paste_line_postamble,
    paste_delay,
    paste_delay_per_lines,
    paste_commented_softcode,
    paste_echo,
    confirm_on_paste,
    send_line_preamble,
    send_line_postamble,
    send_file_delay,
    send_file_delay_per_lines,
    send_commented_softcode,
    send_echo,
    confirm_on_send,
    world_script,
    script_prefix,
    enable_scripts,
    warn_if_scripting_inactive,
    edit_script_with_notepad,
    script_editor,
    script_reload_option,
    script_errors_to_output_window,
    note_text_colour,
    plugins
);

impl Convert<HashMap<String, String>> for Vec<ffi::KeypadMapping> {
    fn from_ffi(value: Self) -> HashMap<String, String> {
        value
            .into_iter()
            .map(|mapping| (mapping.keypad, mapping.command))
            .collect()
    }

    fn to_ffi(value: HashMap<String, String>) -> Self {
        value
            .into_iter()
            .map(|(keypad, command)| ffi::KeypadMapping { keypad, command })
            .collect()
    }
}

impl<'a> From<SendRequest<'a>> for ffi::SendRequest {
    fn from(value: SendRequest<'a>) -> Self {
        Self {
            plugin: value.plugin,
            send_to: value.sender.send_to.into(),
            script: value.sender.script.clone(),
            variable: value.sender.variable.clone(),
            text: value.text.to_owned(),
            wildcards: value.wildcards.into_iter().map(ToOwned::to_owned).collect(),
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

impl Convert<Regex> for String {
    fn from_ffi(value: Self) -> Regex {
        Regex::new(&value).expect("invalid regular expression")
    }

    fn to_ffi(value: Regex) -> Self {
        value.to_string()
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
                Occurrence::Time(NaiveTime::from_hms_opt(h, m, s).unwrap())
            }
        }
    }
}

impl Convert<AudioRepetition> for u32 {
    fn from_ffi(value: Self) -> AudioRepetition {
        match NonZeroU32::new(value) {
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

impl TryFrom<EffectFragment> for ffi::EffectFragment {
    type Error = UnsupportedError;

    fn try_from(value: EffectFragment) -> Result<Self, Self::Error> {
        match value {
            EffectFragment::Backspace => Ok(Self::Backspace),
            EffectFragment::Beep => Ok(Self::Beep),
            EffectFragment::CarriageReturn => Ok(Self::CarriageReturn),
            EffectFragment::EraseCharacter => Ok(Self::EraseCharacter),
            EffectFragment::EraseLine => Ok(Self::EraseLine),
            EffectFragment::ExpireLinks(_) => Err(UnsupportedError("<expire>")),
            EffectFragment::FileFilter(_) => Err(UnsupportedError("<filter>")),
            EffectFragment::Gauge(_) => Err(UnsupportedError("<gauge>")),
            EffectFragment::Music(_) | EffectFragment::MusicOff => Err(UnsupportedError("<music>")),
            EffectFragment::Relocate(_) => Err(UnsupportedError("<relocate>")),
            EffectFragment::Sound(_) | EffectFragment::SoundOff => Err(UnsupportedError("<sound>")),
            EffectFragment::StatusBar(_) => Err(UnsupportedError("<stat>")),
        }
    }
}

impl From<TelnetFragment> for ffi::TelnetFragment {
    fn from(value: TelnetFragment) -> Self {
        match value {
            TelnetFragment::Do { code } => Self::Do { code },
            TelnetFragment::IacGa => Self::IacGa,
            TelnetFragment::Mxp { enabled } => Self::Mxp { enabled },
            TelnetFragment::Naws => Self::Naws,
            TelnetFragment::SetEcho { should_echo } => Self::SetEcho { should_echo },
            TelnetFragment::Subnegotiation { code, data } => Self::Subnegotiation {
                code,
                data: data.to_vec(),
            },
            TelnetFragment::Will { code } => Self::Will { code },
        }
    }
}

impl Clone for ffi::TelnetFragment {
    fn clone(&self) -> Self {
        match self {
            Self::Do { code } => Self::Do { code: *code },
            Self::IacGa => Self::IacGa,
            Self::Mxp { enabled } => Self::Mxp { enabled: *enabled },
            Self::Naws => Self::Naws,
            Self::SetEcho { should_echo } => Self::SetEcho {
                should_echo: *should_echo,
            },
            Self::Subnegotiation { code, data } => Self::Subnegotiation {
                code: *code,
                data: data.clone(),
            },
            Self::Will { code } => Self::Will { code: *code },
        }
    }
}

impl_convert_enum!(ffi::Heading, Heading, H1, H2, H3, H4, H5, H6);

impl TryFrom<OutputFragment> for ffi::OutputFragment {
    type Error = UnsupportedError;

    fn try_from(value: OutputFragment) -> Result<Self, Self::Error> {
        Ok(match value {
            OutputFragment::Effect(effect) => Self::Effect(effect.try_into()?),
            OutputFragment::Frame(_) => return Err(UnsupportedError("<frame>")),
            OutputFragment::Hr => Self::Hr,
            OutputFragment::Image(_) => return Err(UnsupportedError("<image>")),
            OutputFragment::LineBreak => Self::LineBreak,
            OutputFragment::MxpEntity(EntityFragment::Set {
                name,
                value,
                publish,
                is_variable,
            }) => Self::MxpEntitySet {
                name,
                value,
                publish,
                is_variable,
            },
            OutputFragment::MxpEntity(EntityFragment::Unset { name, is_variable }) => {
                Self::MxpEntityUnset { name, is_variable }
            }
            OutputFragment::MxpError(error) => Self::MxpError(error.to_string()),
            OutputFragment::PageBreak => Self::PageBreak,
            OutputFragment::Telnet(telnet) => Self::Telnet(telnet.into()),
            OutputFragment::Text(text) => Self::Text(text.into()),
        })
    }
}
