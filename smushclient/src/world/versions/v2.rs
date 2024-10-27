use std::collections::HashMap;
use std::path::PathBuf;

use mud_transformer::UseMxp;
use serde::Deserialize;
use smushclient_plugins::{Alias, Timer, Trigger};

use mud_transformer::mxp::RgbColor;

use super::super::types::*;

#[derive(Deserialize)]
pub enum ProxyType {
    Socks4,
    Socks5,
}

#[derive(Deserialize)]
pub struct World {
    // IP address
    pub name: String,
    pub site: String,
    pub port: u16,
    pub proxy_type: Option<ProxyType>,
    pub proxy_server: String,
    pub proxy_port: u16,
    pub proxy_username: String,
    pub proxy_password: String,
    pub proxy_password_base64: bool,
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
    pub timers: Vec<Timer>,
    pub enable_timers: bool,
    pub treeview_timers: bool,

    // Chat
    pub chat_name: String,
    pub auto_allow_snooping: bool,
    pub accept_chat_connections: bool,
    pub chat_port: u16,
    pub validate_incoming_chat_calls: bool,
    pub chat_colors: ColorPair,
    pub ignore_chat_colours: bool,
    pub chat_message_prefix: String,
    pub chat_max_lines_per_message: usize,
    pub chat_max_bytes_per_message: usize,
    pub auto_allow_files: bool,
    pub chat_file_save_directory: Option<String>,

    // Notes
    pub notes: String,

    // Output
    pub beep_sound: Option<String>,
    pub pixel_offset: i16,
    pub line_spacing: f64,
    pub output_font: String,
    pub output_font_height: u8,
    pub use_default_output_font: bool,
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub new_activity_sound: Option<String>,
    pub max_output_lines: usize,
    pub wrap_column: u16,

    pub line_information: bool,
    pub start_paused: bool,
    pub auto_pause: bool,
    pub unpause_on_send: bool,
    pub flash_taskbar_icon: bool,
    pub disable_compression: bool,
    pub indent_paras: bool,
    pub naws: bool,
    pub carriage_return_clears_line: bool,
    pub utf_8: bool,
    pub auto_wrap_window_width: bool,
    pub show_connect_disconnect: bool,
    pub copy_selection_to_clipboard: bool,
    pub auto_copy_to_clipboard_in_html: bool,
    pub convert_ga_to_newline: bool,
    pub terminal_identification: String,

    // MXP
    pub use_mxp: UseMxp,
    pub hyperlink_colour: RgbColor,
    pub use_custom_link_colour: bool,
    pub mud_can_change_link_colour: bool,
    pub underline_hyperlinks: bool,
    pub mud_can_remove_underline: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub ignore_mxp_colour_changes: bool,
    pub send_mxp_afk_response: bool,

    // ANSI Color
    pub use_default_colours: bool,
    pub ansi_colors: [RgbColor; 16],

    // Triggers
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,
    pub enable_trigger_sounds: bool,
    pub treeview_triggers: bool,

    // Commands
    pub display_my_input: bool,
    pub echo_colors: ColorPair,
    pub enable_speed_walk: bool,
    pub speed_walk_prefix: String,
    pub speed_walk_filler: String,
    pub speed_walk_delay: f64,
    pub enable_command_stack: bool,
    pub command_stack_character: String,
    pub input_colors: ColorPair,
    pub input_font: String,
    pub input_font_height: u8,
    pub use_default_input_font: bool,
    pub enable_spam_prevention: bool,
    pub spam_line_count: usize,
    pub spam_message: String,

    pub auto_repeat: bool,
    pub lower_case_tab_completion: bool,
    pub translate_german: bool,
    pub translate_backslash_sequences: bool,
    pub keep_commands_on_same_line: bool,
    pub no_echo_off: bool,
    pub tab_completion_lines: usize,
    pub tab_completion_space: bool,

    pub double_click_inserts: bool,
    pub double_click_sends: bool,
    pub escape_deletes_input: bool,
    pub save_deleted_command: bool,
    pub confirm_before_replacing_typing: bool,
    pub arrow_keys_wrap: bool,
    pub arrows_change_history: bool,
    pub arrow_recalls_partial: bool,
    pub alt_arrow_recalls_partial: bool,
    pub ctrl_z_goes_to_end_of_buffer: bool,
    pub ctrl_p_goes_to_previous_command: bool,
    pub ctrl_n_goes_to_next_command: bool,
    pub history_lines: usize,

    // Aliases
    pub aliases: Vec<Alias>,
    pub enable_aliases: bool,
    pub treeview_aliases: bool,

    // Keypad
    pub keypad_enable: bool,
    pub keypad_shortcuts: HashMap<String, String>,

    // Auto Say
    pub enable_auto_say: bool,
    pub autosay_exclude_non_alpha: bool,
    pub autosay_exclude_macros: bool,
    pub auto_say_override_prefix: String,
    pub auto_say_string: String,
    pub re_evaluate_auto_say: bool,

    // Paste
    pub paste_line_preamble: String,
    pub paste_line_postamble: String,
    pub paste_delay: u32,
    pub paste_delay_per_lines: u32,
    pub paste_commented_softcode: bool,
    pub paste_echo: bool,
    pub confirm_on_paste: bool,

    // Send
    pub send_line_preamble: String,
    pub send_line_postamble: String,
    pub send_file_delay: u32,
    pub send_file_delay_per_lines: u32,
    pub send_commented_softcode: bool,
    pub send_echo: bool,
    pub confirm_on_send: bool,

    // Scripts
    pub world_script: String,
    pub script_prefix: String,
    pub enable_scripts: bool,
    pub warn_if_scripting_inactive: bool,
    pub edit_script_with_notepad: bool,
    pub script_editor: String,
    pub script_reload_option: ScriptRecompile,
    pub script_errors_to_output_window: bool,
    pub error_colour: RgbColor,
    pub note_text_colour: RgbColor,

    // Hidden
    pub plugins: Vec<PathBuf>,
}

impl From<World> for super::super::World {
    fn from(value: World) -> Self {
        Self {
            name: value.name,
            site: value.site,
            port: value.port,
            use_proxy: value.proxy_type.is_some(),
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
            notes: value.notes,
            show_bold: value.show_bold,
            show_italic: value.show_italic,
            show_underline: value.show_underline,
            new_activity_sound: value.new_activity_sound,
            disable_compression: value.disable_compression,
            indent_paras: if value.indent_paras { 2 } else { 0 },
            naws: value.naws,
            carriage_return_clears_line: value.carriage_return_clears_line,
            utf_8: value.utf_8,
            convert_ga_to_newline: value.convert_ga_to_newline,
            terminal_identification: value.terminal_identification,
            use_mxp: value.use_mxp,
            hyperlink_colour: value.hyperlink_colour,
            use_custom_link_colour: value.use_custom_link_colour,
            mud_can_change_link_colour: value.mud_can_change_link_colour,
            underline_hyperlinks: value.underline_hyperlinks,
            mud_can_remove_underline: value.mud_can_remove_underline,
            hyperlink_adds_to_command_history: value.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: value.echo_hyperlink_in_output_window,
            ignore_mxp_colour_changes: value.ignore_mxp_colour_changes,
            send_mxp_afk_response: value.send_mxp_afk_response,
            use_default_colours: value.use_default_colours,
            ansi_colors: value.ansi_colors,
            error_colour: value.error_colour,
            triggers: value.triggers,
            enable_triggers: value.enable_triggers,
            display_my_input: value.display_my_input,
            keep_commands_on_same_line: value.keep_commands_on_same_line,
            echo_colors: value.echo_colors,
            enable_speed_walk: value.enable_speed_walk,
            speed_walk_prefix: value.speed_walk_prefix,
            speed_walk_filler: value.speed_walk_filler,
            speed_walk_delay: value.speed_walk_delay,
            enable_command_stack: value.enable_command_stack,
            command_stack_character: value.command_stack_character,
            enable_spam_prevention: value.enable_spam_prevention,
            spam_line_count: value.spam_line_count,
            spam_message: value.spam_message,
            no_echo_off: value.no_echo_off,
            aliases: value.aliases,
            enable_aliases: value.enable_aliases,
            keypad_enable: value.keypad_enable,
            keypad_shortcuts: value.keypad_shortcuts,
            paste_line_preamble: value.paste_line_preamble,
            paste_line_postamble: value.paste_line_postamble,
            paste_delay: value.paste_delay,
            paste_delay_per_lines: value.paste_delay_per_lines,
            paste_echo: value.paste_echo,
            world_script: value.world_script,
            enable_scripts: value.enable_scripts,
            script_reload_option: value.script_reload_option,
            note_text_colour: value.note_text_colour,
            plugins: value.plugins,
        }
    }
}
