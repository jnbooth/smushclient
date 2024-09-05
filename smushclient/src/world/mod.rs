mod error;
pub use error::PersistError;
mod types;
pub use types::*;

use std::collections::HashMap;
use std::io::{Read, Write};

use chrono::Utc;
use mud_transformer::{TransformerConfig, UseMxp};
use serde::{Deserialize, Serialize};
use smushclient_plugins::{Alias, Plugin, PluginMetadata, Sender, Timer, Trigger};

use mud_transformer::mxp::RgbColor;

const CURRENT_VERSION: u8 = 1;

fn skip_temporary<S, T>(vec: &[T], serializer: S) -> Result<S::Ok, S::Error>
where
    S: serde::Serializer,
    T: serde::Serialize + AsRef<Sender>,
{
    // must collect in a vec because bincode needs to know the size ahead of time
    let filtered: Vec<&T> = vec.iter().filter(|x| !x.as_ref().temporary).collect();
    serializer.collect_seq(filtered)
}

#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
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
    #[serde(serialize_with = "skip_temporary")]
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
    pub ignore_chat_colors: bool,
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
    pub output_font_size: u8,
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
    pub hyperlink_color: RgbColor,
    pub use_custom_link_color: bool,
    pub mud_can_change_link_color: bool,
    pub underline_hyperlinks: bool,
    pub mud_can_remove_underline: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub ignore_mxp_color_changes: bool,
    pub send_mxp_afk_response: bool,

    // ANSI Color
    pub use_default_colors: bool,
    pub ansi_colors: [RgbColor; 16],
    pub custom_color: RgbColor,
    pub error_color: RgbColor,

    // Triggers
    #[serde(serialize_with = "skip_temporary")]
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
    pub input_font_size: u8,
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
    #[serde(serialize_with = "skip_temporary")]
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
    pub send_delay: u32,
    pub send_delay_per_lines: u32,
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
    pub note_text_color: RgbColor,

    // Hidden
    pub plugins: Vec<String>,
}

impl Default for World {
    fn default() -> Self {
        Self::new()
    }
}

impl World {
    pub fn new() -> Self {
        Self {
            // IP address
            name: String::new(),
            site: String::new(),
            port: 4000,
            proxy_type: None,
            proxy_server: String::new(),
            proxy_port: 1080,
            proxy_username: String::new(),
            proxy_password: String::new(),
            proxy_password_base64: true,
            save_world_automatically: false,

            // Connecting
            player: String::new(),
            password: String::new(),
            connect_method: None,
            connect_text: String::new(),

            // Logging
            log_file_preamble: String::new(),
            log_file_postamble: String::new(),
            log_format: LogFormat::Text,
            log_output: true,
            log_input: true,
            log_notes: true,
            log_mode: LogMode::Append,
            auto_log_file_name: None,
            log_preamble_output: String::new(),
            log_preamble_input: String::new(),
            log_preamble_notes: String::new(),
            log_postamble_output: String::new(),
            log_postamble_input: String::new(),
            log_postamble_notes: String::new(),

            // Timers
            timers: Vec::new(),
            enable_timers: true,
            treeview_timers: true,

            // Chat
            chat_name: String::new(),
            auto_allow_snooping: false,
            accept_chat_connections: false,
            chat_port: 4050,
            validate_incoming_chat_calls: false,
            chat_colors: ColorPair::foreground(RgbColor::rgb(255, 0, 0)),
            ignore_chat_colors: false,
            chat_message_prefix: String::new(),
            chat_max_lines_per_message: 0,
            chat_max_bytes_per_message: 0,
            auto_allow_files: false,
            chat_file_save_directory: None,

            // Notes
            notes: String::new(),

            // Output
            beep_sound: None,
            pixel_offset: 0,
            line_spacing: 1.0,
            output_font: "System".to_owned(),
            output_font_size: 12,
            use_default_output_font: true,
            show_bold: true,
            show_italic: true,
            show_underline: true,
            new_activity_sound: None,
            max_output_lines: 5000,
            wrap_column: 80,

            line_information: true,
            start_paused: false,
            auto_pause: true,
            unpause_on_send: true,
            flash_taskbar_icon: false,
            disable_compression: false,
            indent_paras: true,
            naws: false,
            carriage_return_clears_line: false,
            utf_8: true,
            auto_wrap_window_width: false,
            show_connect_disconnect: true,
            copy_selection_to_clipboard: false,
            auto_copy_to_clipboard_in_html: false,
            convert_ga_to_newline: false,
            terminal_identification: "mushclient".to_owned(),

            // MXP
            use_mxp: UseMxp::Command,
            hyperlink_color: RgbColor::rgb(43, 121, 162),
            use_custom_link_color: false,
            mud_can_change_link_color: true,
            underline_hyperlinks: true,
            mud_can_remove_underline: false,
            hyperlink_adds_to_command_history: true,
            echo_hyperlink_in_output_window: true,
            ignore_mxp_color_changes: false,
            send_mxp_afk_response: true,

            // ANSI Color
            use_default_colors: true,
            ansi_colors: *RgbColor::XTERM_16,
            custom_color: RgbColor::rgb(0, 164, 152),
            error_color: RgbColor::rgb(127, 0, 0),

            // Triggers
            triggers: Vec::new(),
            enable_triggers: true,
            enable_trigger_sounds: true,
            treeview_triggers: true,

            // Commands
            display_my_input: true,
            echo_colors: ColorPair::foreground(RgbColor::rgb(128, 128, 128)),
            enable_speed_walk: false,
            speed_walk_prefix: "#".to_owned(),
            speed_walk_filler: "a".to_owned(),
            speed_walk_delay: 20.0,
            enable_command_stack: false,
            command_stack_character: "#".to_owned(),
            input_colors: ColorPair::foreground(RgbColor::rgb(128, 128, 128)),
            input_font: "System".to_owned(),
            input_font_size: 12,
            use_default_input_font: true,
            enable_spam_prevention: false,
            spam_line_count: 20,
            spam_message: "look".to_owned(),

            auto_repeat: false,
            lower_case_tab_completion: false,
            translate_german: false,
            translate_backslash_sequences: false,
            keep_commands_on_same_line: false,
            no_echo_off: false,
            tab_completion_lines: 200,
            tab_completion_space: false,

            double_click_inserts: false,
            double_click_sends: false,
            escape_deletes_input: false,
            save_deleted_command: false,
            confirm_before_replacing_typing: true,
            arrow_keys_wrap: false,
            arrows_change_history: true,
            arrow_recalls_partial: false,
            alt_arrow_recalls_partial: false,
            ctrl_z_goes_to_end_of_buffer: false,
            ctrl_p_goes_to_previous_command: false,
            ctrl_n_goes_to_next_command: false,
            history_lines: 1000,

            // Aliases
            aliases: Vec::new(),
            enable_aliases: true,
            treeview_aliases: true,

            // Keypad
            keypad_enable: true,
            keypad_shortcuts: HashMap::new(),

            // Auto Say
            enable_auto_say: false,
            autosay_exclude_non_alpha: false,
            autosay_exclude_macros: false,
            auto_say_override_prefix: "-".to_owned(),
            auto_say_string: String::new(),
            re_evaluate_auto_say: false,

            // Paste
            paste_line_preamble: String::new(),
            paste_line_postamble: String::new(),
            paste_delay: 0,
            paste_delay_per_lines: 1,
            paste_commented_softcode: false,
            paste_echo: false,
            confirm_on_paste: true,

            // Send
            send_line_preamble: String::new(),
            send_line_postamble: String::new(),
            send_delay: 0,
            send_delay_per_lines: 1,
            send_commented_softcode: false,
            send_echo: false,
            confirm_on_send: true,

            // Scripts
            world_script: String::new(),
            script_prefix: String::new(),
            enable_scripts: true,
            warn_if_scripting_inactive: true,
            edit_script_with_notepad: true,
            script_editor: "System".to_owned(),
            script_reload_option: ScriptRecompile::Confirm,
            script_errors_to_output_window: false,
            note_text_color: RgbColor::rgb(0, 128, 255),

            // Hidden
            plugins: Vec::new(),
        }
    }

    pub fn world_plugin(&self) -> Option<Plugin> {
        if self.triggers.is_empty()
            && self.aliases.is_empty()
            && self.timers.is_empty()
            && self.world_script.trim().is_empty()
        {
            return None;
        }
        let today = Utc::now().date_naive();
        let metadata = PluginMetadata {
            name: format!("World Script: {}", self.name),
            written: today,
            modified: today,
            is_world_plugin: true,
            sequence: -1,
            ..Default::default()
        };
        Some(Plugin {
            metadata,
            triggers: self.triggers.clone(),
            aliases: self.aliases.clone(),
            timers: self.timers.clone(),
            script: self.world_script.clone(),
        })
    }

    pub fn save<W: Write>(&self, mut writer: W) -> Result<(), PersistError> {
        writer.write_all(&[CURRENT_VERSION])?;
        bincode::serialize_into(writer, self)?;
        Ok(())
    }

    pub fn load<R: Read>(mut reader: R) -> Result<Self, PersistError> {
        let mut version_buf = [0; 1];
        reader.read_exact(&mut version_buf)?;
        match version_buf[0] {
            1 => bincode::deserialize_from(reader).map_err(Into::into),
            _ => Err(PersistError::NotSave),
        }
    }

    pub fn palette(&self) -> [RgbColor; 166] {
        let mut palette = [RgbColor::BLACK; 166];
        palette[0] = self.custom_color;
        palette[1] = self.error_color;
        palette[2..18].copy_from_slice(&self.ansi_colors);
        for (slot, (_name, color)) in palette[19..].iter_mut().zip(RgbColor::iter_named()) {
            *slot = color;
        }
        palette
    }
}

impl From<&World> for TransformerConfig {
    fn from(value: &World) -> Self {
        Self {
            use_mxp: value.use_mxp,
            disable_compression: value.disable_compression,
            terminal_identification: value.terminal_identification.clone(),
            player: value.player.clone(),
            password: value.password.clone(),
            convert_ga_to_newline: value.convert_ga_to_newline,
            no_echo_off: value.no_echo_off,
            naws: value.naws,
            disable_utf8: !value.utf_8,
            ignore_mxp_colors: value.ignore_mxp_color_changes,
            colors: value.ansi_colors.to_vec(),
            ..Default::default()
        }
    }
}
