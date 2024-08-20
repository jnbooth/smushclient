use std::collections::HashMap;
use std::pin::Pin;

use cxx_qt_lib::{QColor, QString, QStringList, QVector};
use smushclient_plugins::{Alias, Timer, Trigger};

use crate::ffi;

#[derive(Default)]
pub struct WorldRust {
    // IP address
    pub name: QString,
    pub site: QString,
    pub port: u16,
    pub proxy_type: ffi::ProxyType,
    pub proxy_server: QString,
    pub proxy_port: u16,
    pub proxy_username: QString,
    pub proxy_password: QString,
    pub proxy_password_base64: bool,
    pub save_world_automatically: bool,

    // Connecting
    pub player: QString,
    pub password: QString,
    pub connect_method: ffi::AutoConnect,
    pub connect_text: QString,

    // Logging
    pub log_file_preamble: QString,
    pub log_file_postamble: QString,
    pub log_format: ffi::LogFormat,
    pub log_output: bool,
    pub log_input: bool,
    pub log_notes: bool,
    pub log_mode: ffi::LogMode,
    pub auto_log_file_name: QString,
    pub log_preamble_output: QString,
    pub log_preamble_input: QString,
    pub log_preamble_notes: QString,
    pub log_postamble_output: QString,
    pub log_postamble_input: QString,
    pub log_postamble_notes: QString,

    // Timers
    pub timers: Vec<Timer>,
    pub enable_timers: bool,
    pub treeview_timers: bool,

    // Chat
    pub chat_name: QString,
    pub auto_allow_snooping: bool,
    pub accept_chat_connections: bool,
    pub chat_port: u16,
    pub validate_incoming_chat_calls: bool,
    pub chat_color_fg: QColor,
    pub chat_color_bg: QColor,
    pub ignore_chat_colors: bool,
    pub chat_message_prefix: QString,
    pub chat_max_lines_per_message: usize,
    pub chat_max_bytes_per_message: usize,
    pub auto_allow_files: bool,
    pub chat_file_save_directory: QString,

    // Notes
    pub notes: QString,

    // Output
    pub beep_sound: QString,
    pub pixel_offset: i16,
    pub line_spacing: f32,
    pub output_font: QString,
    pub use_default_output_font: bool,
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub new_activity_sound: QString,
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
    pub terminal_identification: QString,

    // MXP / Pueblo
    pub use_mxp: ffi::UseMxp,
    pub detect_pueblo: bool,
    pub hyperlink_color: QColor,
    pub use_custom_link_color: bool,
    pub mud_can_change_link_color: bool,
    pub underline_hyperlinks: bool,
    pub mud_can_remove_underline: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub ignore_mxp_color_changes: bool,
    pub send_mxp_afk_response: bool,
    pub mud_can_change_options: bool,

    // ANSI Color
    pub use_default_colors: bool,
    pub ansi_colors: QVector<QColor>,

    // Custom Color
    pub custom_names: QStringList,
    pub custom_color_fgs: QVector<QColor>,
    pub custom_color_bgs: QVector<QColor>,

    // Triggers
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,
    pub enable_trigger_sounds: bool,
    pub treeview_triggers: bool,

    // Commands
    pub display_my_input: bool,
    pub echo_color_fg: QColor,
    pub echo_color_bg: QColor,
    pub enable_speed_walk: bool,
    pub speed_walk_prefix: QString,
    pub speed_walk_filler: QString,
    pub speed_walk_delay: u32,
    pub enable_command_stack: bool,
    pub command_stack_character: QString,
    pub input_color_fg: QColor,
    pub input_color_bg: QColor,
    pub input_font: QString,
    pub use_default_input_font: bool,
    pub enable_spam_prevention: bool,
    pub spam_line_count: usize,
    pub spam_message: QString,

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
    pub auto_say_override_prefix: QString,
    pub auto_say_string: QString,
    pub re_evaluate_auto_say: bool,

    // Paste
    pub paste_line_preamble: QString,
    pub paste_line_postamble: QString,
    pub paste_delay: u32,
    pub paste_delay_per_lines: u32,
    pub paste_commented_softcode: bool,
    pub paste_echo: bool,
    pub confirm_on_paste: bool,

    // Send
    pub send_line_preamble: QString,
    pub send_line_postamble: QString,
    pub send_delay: u32,
    pub send_delay_per_lines: u32,
    pub send_commented_softcode: bool,
    pub send_echo: bool,
    pub confirm_on_send: bool,

    // Scripts
    pub world_script: QString,
    pub script_prefix: QString,
    pub enable_scripts: bool,
    pub warn_if_scripting_inactive: bool,
    pub edit_script_with_notepad: bool,
    pub script_editor: QString,
    pub script_reload_option: ffi::ScriptRecompile,
    pub script_errors_to_output_window: bool,
    pub note_text_color: QColor,

    pub plugins: QStringList,
}

impl WorldRust {
    pub fn populate(&mut self, client: &ffi::SmushClient) {
        let world = client.world();
        self.name = QString::from(&world.name);
        self.site = QString::from(&world.site);
        self.port = world.port;
    }
}

impl ffi::World {
    pub fn populate(self: Pin<&mut Self>, client: &ffi::SmushClient) {
        self.cxx_qt_ffi_rust_mut().populate(client);
    }
}
