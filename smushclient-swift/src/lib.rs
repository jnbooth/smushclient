#![allow(clippy::expl_impl_clone_on_copy)]
#![allow(clippy::ptr_as_ptr)]
#![allow(clippy::unnecessary_cast)]
#![allow(clippy::not_unsafe_ptr_arg_deref)]

#[macro_use]
extern crate enumeration;

#[macro_use]
mod convert;

mod bridge;
mod client;
mod error;
mod impls;
mod io;
mod output;
mod sync;

use crate::bridge::{RustAliasOutcome, RustMudBridge, RustOutputStream, RustSendStream};
use crate::io::{create_world, read_world, write_world};
use crate::output::{RustMxpLink, RustTextFragment};

fn convert_opt<Ffi, Rust: Into<Ffi>>(from: Option<Rust>) -> Option<Ffi> {
    from.map(Into::into)
}

fn convert_result<Ffi, Rust: Into<Ffi>>(from: Result<Rust, String>) -> Result<Ffi, String> {
    from.map(Into::into)
}

fn convert_world<T: Into<ffi::World>>(from: Result<T, String>) -> Result<ffi::World, String> {
    convert_result(from)
}

#[swift_bridge::bridge]
pub mod ffi {
    #[derive(Copy, Clone)]
    #[swift_bridge(swift_repr = "struct")]
    struct RgbColor {
        r: u8,
        g: u8,
        b: u8,
    }

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

    enum SendTo {
        Internet,
        World,
        Input,
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
    }

    enum TelnetFragment {
        Do { code: u8 },
        IacGa,
        Naws,
        SetEcho { should_echo: bool },
        Subnegotiation { code: u8, data: Vec<u8> },
        Will { code: u8 },
    }

    enum ColorOption {
        None,
        Some(RgbColor),
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
        line_spacing: f64,
        output_font: String,
        output_font_size: u8,
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
        hyperlink_color: RgbColor,
        use_custom_link_color: bool,
        mud_can_change_link_color: bool,
        underline_hyperlinks: bool,
        mud_can_remove_underline: bool,
        hyperlink_adds_to_command_history: bool,
        echo_hyperlink_in_output_window: bool,
        ignore_mxp_color_changes: bool,
        send_mxp_afk_response: bool,

        use_default_colors: bool,
        ansi_colors: Vec<RgbColor>,
        custom_color: RgbColor,
        error_color: RgbColor,

        triggers: Vec<Trigger>,
        enable_triggers: bool,
        enable_trigger_sounds: bool,
        treeview_triggers: bool,

        display_my_input: bool,
        echo_colors: ColorPair,
        enable_speed_walk: bool,
        speed_walk_prefix: String,
        speed_walk_filler: String,
        speed_walk_delay: f64,
        enable_command_stack: bool,
        command_stack_character: String,
        input_colors: ColorPair,
        input_font: String,
        input_font_size: u8,
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
        note_text_color: RgbColor,

        plugins: Vec<String>,
    }

    enum Heading {
        H1,
        H2,
        H3,
        H4,
        H5,
        H6,
    }

    extern "Rust" {
        type RustMxpLink;
        fn action(&self) -> &str;
        fn hint(&self) -> Option<&str>;
        fn prompts(&self) -> Vec<String>;
        #[swift_bridge(return_into)]
        fn sendto(&self) -> SendTo;
    }

    extern "Rust" {
        type RustTextFragment;
        fn text(&self) -> &str;
        #[swift_bridge(return_into)]
        fn foreground(&self) -> RgbColor;
        #[swift_bridge(return_into)]
        fn background(&self) -> RgbColor;
        #[swift_bridge(swift_name = "isBlink")]
        fn is_blink(&self) -> bool;
        #[swift_bridge(swift_name = "isBold")]
        fn is_bold(&self) -> bool;
        #[swift_bridge(swift_name = "isHighlight")]
        fn is_highlight(&self) -> bool;
        #[swift_bridge(swift_name = "isItalic")]
        fn is_italic(&self) -> bool;
        #[swift_bridge(swift_name = "isStrikeout")]
        fn is_strikeout(&self) -> bool;
        #[swift_bridge(swift_name = "isUnderline")]
        fn is_underline(&self) -> bool;
        fn link(&self) -> Option<&RustMxpLink>;
        #[swift_bridge(return_with = convert_opt)]
        fn heading(&self) -> Option<Heading>;
    }

    enum OutputFragment {
        Effect(EffectFragment),
        Hr,
        LineBreak,
        MxpError(String),
        MxpEntitySet {
            name: String,
            value: String,
            publish: bool,
            is_variable: bool,
        },
        MxpEntityUnset {
            name: String,
            is_variable: bool,
        },
        PageBreak,
        Send(SendRequest),
        Sound(String),
        Telnet(TelnetFragment),
        Text(RustTextFragment),
    }

    extern "Rust" {
        type RustSendStream;
        fn next(&mut self) -> Option<SendRequest>;
        fn count(&self) -> usize;
    }

    extern "Rust" {
        type RustOutputStream;
        fn next(&mut self) -> Option<OutputFragment>;
        fn count(&self) -> usize;
    }

    extern "Rust" {
        type RustAliasOutcome;
        fn should_display(&self) -> bool;
        fn should_remember(&self) -> bool;
        fn should_send(&self) -> bool;
        fn stream(&mut self) -> RustSendStream;
    }

    extern "Rust" {
        type RustMudBridge;
        #[swift_bridge(init, args_into = (world))]
        fn new(world: World) -> RustMudBridge;
        #[swift_bridge(associated_to = RustMudBridge)]
        fn load(path: String) -> Result<RustMudBridge, String>;
        fn save(&self, path: String) -> Result<(), String>;
        #[swift_bridge(return_into)]
        fn world(&self) -> World;
        #[swift_bridge(args_into = (world))]
        fn set_world(&mut self, world: World);
        fn connected(&self) -> bool;
        fn alias(&mut self, command: String) -> RustAliasOutcome;
        async fn connect(&mut self) -> Result<(), String>;
        async fn disconnect(&mut self) -> Result<(), String>;
        async fn receive(&mut self) -> Result<RustOutputStream, String>;
        async fn send(&mut self, input: String) -> Result<(), String>;
    }

    extern "Rust" {
        #[swift_bridge(swift_name = "createWorld", return_into)]
        fn create_world() -> World;
        #[swift_bridge(swift_name = "readWorld", return_with = convert_world)]
        fn read_world(data: &[u8]) -> Result<World, String>;
        #[swift_bridge(swift_name = "writeWorld", args_into = (world))]
        fn write_world(world: World) -> Result<Vec<u8>, String>;
    }
}

impl Clone for ffi::ColorOption {
    fn clone(&self) -> Self {
        *self
    }
}

impl Copy for ffi::ColorOption {}

impl Clone for ffi::Occurrence {
    fn clone(&self) -> Self {
        *self
    }
}

impl Copy for ffi::Occurrence {}
