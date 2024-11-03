use std::path::PathBuf;

use mud_transformer::UseMxp;
use serde::Deserialize;
use smushclient_plugins::{Alias, Timer, Trigger};

use mud_transformer::mxp::RgbColor;

use super::super::types::*;

#[derive(Deserialize)]
pub struct World {
    // IP address
    pub name: String,
    pub site: String,
    pub port: u16,
    pub use_proxy: bool,
    pub proxy_server: String,
    pub proxy_port: u16,
    pub proxy_username: String,
    pub proxy_password: String,
    pub save_world_automatically: bool,

    // Connecting
    pub player: String,
    pub password: String,
    pub connect_method: Option<AutoConnect>,
    pub connect_text: String,

    // Logging
    pub log_file_preamble: String,
    pub log_file_postamble: String,
    pub log_format: LogFormat,
    pub log_output: bool,
    pub log_input: bool,
    pub log_notes: bool,
    pub log_mode: LogMode,
    pub auto_log_file_name: Option<String>,
    pub log_preamble_output: String,
    pub log_preamble_input: String,
    pub log_preamble_notes: String,
    pub log_postamble_output: String,
    pub log_postamble_input: String,
    pub log_postamble_notes: String,

    // Timers
    #[serde(serialize_with = "skip_temporary")]
    pub timers: Vec<Timer>,
    pub enable_timers: bool,

    // Output
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub indent_paras: u8,
    pub ansi_colors: [RgbColor; 16],
    pub new_activity_sound: Option<String>,

    // MUD
    pub use_mxp: UseMxp,
    pub ignore_mxp_colour_changes: bool,
    pub use_custom_link_colour: bool,
    pub hyperlink_colour: RgbColor,
    pub mud_can_change_link_colour: bool,
    pub underline_hyperlinks: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub terminal_identification: String,
    pub disable_compression: bool,
    pub naws: bool,
    pub carriage_return_clears_line: bool,
    pub utf_8: bool,
    pub convert_ga_to_newline: bool,

    // Triggers
    #[serde(serialize_with = "skip_temporary")]
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,

    // Commands
    pub display_my_input: bool,
    pub echo_colors: ColorPair,
    pub keep_commands_on_same_line: bool,
    pub no_echo_off: bool,
    pub command_queue_delay: f64,
    pub enable_command_stack: bool,
    pub command_stack_character: u16,
    pub enable_speed_walk: bool,
    pub speed_walk_prefix: String,
    pub speed_walk_filler: String,
    pub enable_spam_prevention: bool,
    pub spam_line_count: usize,
    pub spam_message: String,

    // Aliases
    #[serde(serialize_with = "skip_temporary")]
    pub aliases: Vec<Alias>,
    pub enable_aliases: bool,

    // Numpad
    pub numpad_shortcuts: NumpadMapping,
    pub numpad_enable: bool,
    pub hotkey_adds_to_command_history: bool,
    pub echo_hotkey_in_output_window: bool,

    // Scripts
    pub enable_scripts: bool,
    pub world_script: Option<String>,
    pub script_reload_option: ScriptRecompile,
    pub note_text_colour: RgbColor,
    pub error_colour: RgbColor,

    // Hidden
    pub plugins: Vec<PathBuf>,
}

impl From<World> for super::super::World {
    fn from(value: World) -> Self {
        Self {
            name: value.name,
            site: value.site,
            port: value.port,
            use_proxy: value.use_proxy,
            proxy_server: value.proxy_server,
            proxy_port: value.proxy_port,
            proxy_username: value.proxy_username,
            proxy_password: value.proxy_password,
            save_world_automatically: value.save_world_automatically,

            player: value.player,
            password: value.password,
            connect_method: value.connect_method,
            connect_text: value.connect_text,

            log_file_preamble: value.log_file_preamble,
            log_file_postamble: value.log_file_postamble,
            log_format: value.log_format,
            log_output: value.log_output,
            log_input: value.log_input,
            log_notes: value.log_notes,
            log_mode: value.log_mode,
            auto_log_file_name: value.auto_log_file_name,
            log_preamble_output: value.log_preamble_output,
            log_preamble_input: value.log_preamble_input,
            log_preamble_notes: value.log_preamble_notes,
            log_postamble_output: value.log_postamble_output,
            log_postamble_input: value.log_postamble_input,
            log_postamble_notes: value.log_postamble_notes,

            timers: value.timers,
            enable_timers: value.enable_timers,

            show_bold: value.show_bold,
            show_italic: value.show_italic,
            show_underline: value.show_underline,
            indent_paras: value.indent_paras,
            ansi_colors: value.ansi_colors,
            display_my_input: value.display_my_input,
            keep_commands_on_same_line: value.keep_commands_on_same_line,
            echo_colors: value.echo_colors,
            new_activity_sound: value.new_activity_sound,

            use_mxp: value.use_mxp,
            ignore_mxp_colour_changes: value.ignore_mxp_colour_changes,
            use_custom_link_colour: value.use_custom_link_colour,
            hyperlink_colour: value.hyperlink_colour,
            mud_can_change_link_colour: value.mud_can_change_link_colour,
            underline_hyperlinks: value.underline_hyperlinks,
            hyperlink_adds_to_command_history: value.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: value.echo_hyperlink_in_output_window,
            terminal_identification: value.terminal_identification,
            disable_compression: value.disable_compression,
            naws: value.naws,
            carriage_return_clears_line: value.carriage_return_clears_line,
            utf_8: value.utf_8,
            convert_ga_to_newline: value.convert_ga_to_newline,
            no_echo_off: value.no_echo_off,
            enable_command_stack: value.enable_command_stack,
            command_stack_character: value.command_stack_character,

            triggers: value.triggers,
            enable_triggers: value.enable_triggers,

            aliases: value.aliases,
            enable_aliases: value.enable_aliases,

            numpad_shortcuts: NumpadMapping::navigation(),
            numpad_enable: value.numpad_enable,
            hotkey_adds_to_command_history: false,
            echo_hotkey_in_output_window: true,

            enable_scripts: value.enable_scripts,
            world_script: None,
            script_reload_option: value.script_reload_option,
            note_text_colour: value.note_text_colour,
            error_colour: value.error_colour,

            plugins: value.plugins,
        }
    }
}
