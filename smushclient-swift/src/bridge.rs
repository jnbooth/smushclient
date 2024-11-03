#![allow(clippy::expl_impl_clone_on_copy)]
#![allow(clippy::ptr_as_ptr)]
#![allow(clippy::unnecessary_cast)]
#![allow(clippy::not_unsafe_ptr_arg_deref)]
#![allow(dead_code)]

use crate::io::{create_world, read_world, write_world};
use crate::mud::RustMudBridge;
use crate::output::{RustMxpLink, RustNamedColorIter, RustTextFragment};
use crate::stream::{RustAliasOutcome, RustOutputStream, RustSendStream};

fn convert_opt<Ffi, Rust: Into<Ffi>>(from: Option<Rust>) -> Option<Ffi> {
    from.map(Into::into)
}

fn convert_result<Ffi, Rust: Into<Ffi>>(from: Result<Rust, String>) -> Result<Ffi, String> {
    from.map(Into::into)
}

fn convert_world<T: Into<ffi::World>>(from: Result<T, String>) -> Result<ffi::World, String> {
    convert_result(from)
}

#[allow(non_camel_case_types)]
#[swift_bridge::bridge]
pub mod ffi {
    #[derive(Copy, Clone)]
    #[swift_bridge(swift_repr = "struct")]
    struct RgbColor {
        r: u8,
        g: u8,
        b: u8,
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

    enum TelnetSource {
        Client,
        Server,
    }

    enum TelnetVerb {
        Do,
        Dont,
        Will,
        Wont,
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
    }

    enum TelnetFragment {
        IacGa,
        Mxp {
            enabled: bool,
        },
        Naws,
        Negotiation {
            source: TelnetSource,
            verb: TelnetVerb,
            code: u8,
        },
        SetEcho {
            should_echo: bool,
        },
        Subnegotiation {
            code: u8,
            data: Vec<u8>,
        },
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

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct SendRequest {
        plugin: usize,
        send_to: SendTarget,
        text: String,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct SendScriptRequest {
        plugin: usize,
        script: String,
        label: String,
        line: String,
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
        lock: bool,
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
        foreground_color: RgbColor,
        change_background: bool,
        background_color: RgbColor,
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
        id: u16,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct Numpad {
        key_0: String,
        key_1: String,
        key_2: String,
        key_3: String,
        key_4: String,
        key_5: String,
        key_6: String,
        key_7: String,
        key_8: String,
        key_9: String,
        key_period: String,
        key_slash: String,
        key_asterisk: String,
        key_minus: String,
        key_plus: String,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct NumpadMapping {
        base: Numpad,
        modified: Numpad,
    }

    #[swift_bridge(swift_repr = "struct")]
    #[derive(Clone)]
    struct World {
        name: String,
        site: String,
        port: u16,
        use_proxy: bool,
        proxy_server: String,
        proxy_port: u16,
        proxy_username: String,
        proxy_password: String,
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

        show_bold: bool,
        show_italic: bool,
        show_underline: bool,
        ansi_colors: Vec<RgbColor>,
        indent_paras: u8,
        display_my_input: bool,
        echo_colors: ColorPair,
        keep_commands_on_same_line: bool,
        new_activity_sound: String,

        use_mxp: UseMxp,
        ignore_mxp_colour_changes: bool,
        use_custom_link_colour: bool,
        hyperlink_colour: RgbColor,
        mud_can_change_link_colour: bool,
        underline_hyperlinks: bool,
        hyperlink_adds_to_command_history: bool,
        echo_hyperlink_in_output_window: bool,
        terminal_identification: String,
        disable_compression: bool,
        naws: bool,
        carriage_return_clears_line: bool,
        utf_8: bool,
        convert_ga_to_newline: bool,
        no_echo_off: bool,
        enable_command_stack: bool,
        command_stack_character: u16,

        triggers: Vec<Trigger>,
        enable_triggers: bool,

        aliases: Vec<Alias>,
        enable_aliases: bool,

        numpad_shortcuts: NumpadMapping,
        numpad_enable: bool,
        hotkey_adds_to_command_history: bool,
        echo_hotkey_in_output_window: bool,

        enable_scripts: bool,
        world_script: String,
        script_reload_option: ScriptRecompile,
        note_text_colour: RgbColor,
        error_colour: RgbColor,

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
        type RustNamedColorIter;
        #[swift_bridge(init)]
        fn new() -> RustNamedColorIter;
        fn next(&mut self) -> Option<RgbColor>;
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
