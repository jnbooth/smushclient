use crate::convert::Convert;
use std::collections::HashMap;
use std::time::Duration;

use mud_transformer::mxp::{HexColor, WorldColor};
use mud_transformer::UseMxp;
use smushclient::world::{AutoConnect, ColorPair, LogFormat, LogMode, ProxyType, ScriptRecompile};
use smushclient::{SendRequest, World};
use smushclient_plugins::{
    Alias, NaiveTime, Occurrence, Reaction, Regex, SendTarget, Sender, Timelike, Timer, Trigger,
};

#[swift_bridge::bridge]
pub mod ffi {
    enum ProxyType {
        None,
        Socks4,
        Socks5,
    }

    enum AutoConnect {
        None,
        Mush,
        Diku,
        Mxp,
    }

    enum ScriptRecompile {
        Confirm,
        Always,
        Never,
    }

    enum LogFormat {
        Text,
        Html,
        Raw,
    }

    enum LogMode {
        Append,
        Overwrite,
    }

    enum UseMxp {
        Command,
        Query,
        Always,
        Never,
    }

    #[derive(Copy, Clone)]
    #[swift_bridge(swift_repr = "struct")]
    struct HexColor {
        r: u8,
        g: u8,
        b: u8,
    }

    enum MudColor {
        Ansi(u8),
        Hex(HexColor),
    }

    enum ColorOption {
        None,
        Some(HexColor),
    }

    #[derive(Copy, Clone)]
    #[swift_bridge(swift_repr = "struct")]
    struct ColorPair {
        foreground: ColorOption,
        background: ColorOption,
    }

    enum Occurrence {
        Interval { s: u64 },
        Time { h: u32, m: u32, s: u32 },
    }

    enum SendTarget {
        World,
        WorldDelay,
        WorldImmediate,
        Command,
        Output,
        Status,
        NotepadNew,
        NotepadAppend,
        NotepadReplace,
        Log,
        Speedwalk,
        Execute,
        Variable,
        Script,
        ScriptAfterOmit,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct SendRequest {
        plugin: usize,
        send_to: SendTarget,
        script: String,
        variable: String,
        text: String,
        wildcards: Vec<String>,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct Sender {
        send_to: SendTarget,
        label: String,
        script: String,
        group: String,
        variable: String,
        text: String,
        enabled: bool,
        one_shot: bool,
        temporary: bool,
        omit_from_output: bool,
        omit_from_log: bool,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct Reaction {
        sequence: i16,
        pattern: String,
        send: Sender,
        ignore_case: bool,
        keep_evaluating: bool,
        is_regex: bool,
        expand_variables: bool,
        repeats: bool,
        regex: String,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct Alias {
        reaction: Reaction,
        echo_alias: bool,
        menu: bool,
        omit_from_command_history: bool,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct Trigger {
        reaction: Reaction,
        change_foreground: bool,
        foreground: String,
        foreground_color: ColorOption,
        change_background: bool,
        background: String,
        background_color: ColorOption,
        make_bold: bool,
        make_italic: bool,
        make_underline: bool,
        sound: String,
        sound_if_inactive: bool,
        lowercase_wildcard: bool,
        multi_line: bool,
        lines_to_match: u8,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct Timer {
        send: Sender,
        occurrence: Occurrence,
        active_closed: bool,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct KeypadMapping {
        keypad: String,
        command: String,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct World {
        name: String,
        site: String,
        port: u16,
        proxy_type: ProxyType,
        proxy_server: String,
        proxy_port: u16,
        proxy_username: String,
        proxy_password: String,
        proxy_password_base64: bool,
        save_world_automatically: bool,

        player: String,
        password: String,
        connect_method: AutoConnect,
        connect_text: String,

        log_file_preamble: String,
        log_file_postamble: String,
        log_format: LogFormat,
        log_output: bool,
        log_input: bool,
        log_notes: bool,
        log_mode: LogMode,
        auto_log_file_name: String,
        log_preamble_output: String,
        log_preamble_input: String,
        log_preamble_notes: String,
        log_postamble_output: String,
        log_postamble_input: String,
        log_postamble_notes: String,

        timers: Vec<Timer>,
        enable_timers: bool,
        treeview_timers: bool,

        chat_name: String,
        auto_allow_snooping: bool,
        accept_chat_connections: bool,
        chat_port: u16,
        validate_incoming_chat_calls: bool,
        chat_colors: ColorPair,
        ignore_chat_colors: bool,
        chat_message_prefix: String,
        chat_max_lines_per_message: usize,
        chat_max_bytes_per_message: usize,
        auto_allow_files: bool,
        chat_file_save_directory: String,
        notes: String,

        beep_sound: String,
        pixel_offset: i16,
        line_spacing: f32,
        output_font: String,
        use_default_output_font: bool,
        show_bold: bool,
        show_italic: bool,
        show_underline: bool,
        new_activity_sound: String,
        max_output_lines: usize,
        wrap_column: u16,

        line_information: bool,
        start_paused: bool,
        auto_pause: bool,
        unpause_on_send: bool,
        flash_taskbar_icon: bool,
        disable_compression: bool,
        indent_paras: bool,
        naws: bool,
        carriage_return_clears_line: bool,
        utf_8: bool,
        auto_wrap_window_width: bool,
        show_connect_disconnect: bool,
        copy_selection_to_clipboard: bool,
        auto_copy_to_clipboard_in_html: bool,
        convert_ga_to_newline: bool,
        terminal_identification: String,

        use_mxp: UseMxp,
        detect_pueblo: bool,
        hyperlink_color: HexColor,
        use_custom_link_color: bool,
        mud_can_change_link_color: bool,
        underline_hyperlinks: bool,
        mud_can_remove_underline: bool,
        hyperlink_adds_to_command_history: bool,
        echo_hyperlink_in_output_window: bool,
        ignore_mxp_color_changes: bool,
        send_mxp_afk_response: bool,
        mud_can_change_options: bool,

        use_default_colors: bool,
        ansi_colors: Vec<HexColor>,

        custom_names: Vec<String>,
        custom_colors: Vec<ColorPair>,

        triggers: Vec<Trigger>,
        enable_triggers: bool,
        enable_trigger_sounds: bool,
        treeview_triggers: bool,

        display_my_input: bool,
        echo_colors: ColorPair,
        enable_speed_walk: bool,
        speed_walk_prefix: String,
        speed_walk_filler: String,
        speed_walk_delay: u32,
        enable_command_stack: bool,
        command_stack_character: String,
        input_colors: ColorPair,
        input_font: String,
        use_default_input_font: bool,
        enable_spam_prevention: bool,
        spam_line_count: usize,
        spam_message: String,

        auto_repeat: bool,
        lower_case_tab_completion: bool,
        translate_german: bool,
        translate_backslash_sequences: bool,
        keep_commands_on_same_line: bool,
        no_echo_off: bool,
        tab_completion_lines: usize,
        tab_completion_space: bool,

        double_click_inserts: bool,
        double_click_sends: bool,
        escape_deletes_input: bool,
        save_deleted_command: bool,
        confirm_before_replacing_typing: bool,
        arrow_keys_wrap: bool,
        arrows_change_history: bool,
        arrow_recalls_partial: bool,
        alt_arrow_recalls_partial: bool,
        ctrl_z_goes_to_end_of_buffer: bool,
        ctrl_p_goes_to_previous_command: bool,
        ctrl_n_goes_to_next_command: bool,
        history_lines: usize,

        aliases: Vec<Alias>,
        enable_aliases: bool,
        treeview_aliases: bool,

        keypad_enable: bool,
        keypad_shortcuts: Vec<KeypadMapping>,
        enable_auto_say: bool,
        autosay_exclude_non_alpha: bool,
        autosay_exclude_macros: bool,
        auto_say_override_prefix: String,
        auto_say_string: String,
        re_evaluate_auto_say: bool,

        paste_line_preamble: String,
        paste_line_postamble: String,
        paste_delay: u32,
        paste_delay_per_lines: u32,
        paste_commented_softcode: bool,
        paste_echo: bool,
        confirm_on_paste: bool,

        send_line_preamble: String,
        send_line_postamble: String,
        send_delay: u32,
        send_delay_per_lines: u32,
        send_commented_softcode: bool,
        send_echo: bool,
        confirm_on_send: bool,

        world_script: String,
        script_prefix: String,
        enable_scripts: bool,
        warn_if_scripting_inactive: bool,
        edit_script_with_notepad: bool,
        script_editor: String,
        script_reload_option: ScriptRecompile,
        script_errors_to_output_window: bool,
        note_text_color: HexColor,

        plugins: Vec<String>,
    }
}

impl Copy for ffi::ColorOption {}

impl Copy for ffi::MudColor {}

impl Clone for ffi::MudColor {
    fn clone(&self) -> Self {
        *self
    }
}

impl From<WorldColor> for ffi::MudColor {
    #[inline]
    fn from(value: WorldColor) -> Self {
        match value {
            WorldColor::Ansi(code) => Self::Ansi(code),
            WorldColor::Hex(color) => Self::Hex(color.into()),
        }
    }
}

impl From<ffi::MudColor> for WorldColor {
    fn from(value: ffi::MudColor) -> Self {
        match value {
            ffi::MudColor::Ansi(code) => Self::Ansi(code),
            ffi::MudColor::Hex(color) => Self::Hex(color.into()),
        }
    }
}

impl_convert!(ffi::MudColor, WorldColor);

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
    WorldDelay,
    WorldImmediate,
    Command,
    Output,
    Status,
    NotepadNew,
    NotepadAppend,
    NotepadReplace,
    Log,
    Speedwalk,
    Execute,
    Variable,
    Script,
    ScriptAfterOmit
);

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

impl From<HexColor> for ffi::HexColor {
    fn from(value: HexColor) -> Self {
        Self {
            r: value.r(),
            g: value.g(),
            b: value.b(),
        }
    }
}

impl From<ffi::HexColor> for HexColor {
    fn from(value: ffi::HexColor) -> Self {
        HexColor::rgb(value.r, value.g, value.b)
    }
}

impl_convert!(ffi::HexColor, HexColor);

impl Clone for ffi::ColorOption {
    fn clone(&self) -> Self {
        *self
    }
}

impl Clone for ffi::Occurrence {
    fn clone(&self) -> Self {
        *self
    }
}

impl Copy for ffi::Occurrence {}

impl From<ffi::ColorOption> for Option<HexColor> {
    fn from(value: ffi::ColorOption) -> Self {
        match value {
            ffi::ColorOption::Some(color) => Some(color.into()),
            ffi::ColorOption::None => None,
        }
    }
}

impl From<Option<HexColor>> for ffi::ColorOption {
    fn from(value: Option<HexColor>) -> Self {
        match value {
            Some(color) => ffi::ColorOption::Some(color.into()),
            None => ffi::ColorOption::None,
        }
    }
}

impl_convert!(ffi::ColorOption, Option<HexColor>);

impl_convert_struct!(ffi::ColorPair, ColorPair, foreground, background,);

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

impl Convert<Regex> for String {
    fn from_ffi(value: Self) -> Regex {
        Regex::new(&value).expect("invalid regular expression")
    }

    fn to_ffi(value: Regex) -> Self {
        value.to_string()
    }
}

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
    foreground,
    foreground_color,
    change_background,
    background,
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

impl_convert!(ffi::Occurrence, Occurrence);

impl_convert_struct!(ffi::Timer, Timer, send, occurrence, active_closed);

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
    ignore_chat_colors,
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
    detect_pueblo,
    hyperlink_color,
    use_custom_link_color,
    mud_can_change_link_color,
    underline_hyperlinks,
    mud_can_remove_underline,
    hyperlink_adds_to_command_history,
    echo_hyperlink_in_output_window,
    ignore_mxp_color_changes,
    send_mxp_afk_response,
    mud_can_change_options,
    use_default_colors,
    ansi_colors,
    custom_names,
    custom_colors,
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
    send_delay,
    send_delay_per_lines,
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
    note_text_color,
    plugins
);
