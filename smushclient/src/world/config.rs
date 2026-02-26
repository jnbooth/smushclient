use std::borrow::Cow;
use std::cell::Cell;
use std::path::PathBuf;

use chrono::Local;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{TransformerConfig, UseMxp};
use serde::{Deserialize, Serialize};
use smushclient_plugins::{Plugin, PluginMetadata};
use uuid::Uuid;

use super::escaping::{Escaped, LogBrackets};
#[allow(clippy::wildcard_imports)]
use super::types::*;

#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct WorldConfig {
    // Connecting
    pub name: String,
    pub site: String,
    pub port: u16,
    pub use_ssl: bool,
    pub use_proxy: bool,
    pub proxy_server: String,
    pub proxy_port: u16,
    pub proxy_username: String,
    pub proxy_password: String,
    pub save_world_automatically: bool,

    // Login
    pub player: String,
    pub password: String,
    pub connect_method: AutoConnect,
    pub connect_text: String,

    // Logging
    pub log_file_preamble: String,
    pub log_file_postamble: String,
    pub log_format: LogFormat,
    pub log_in_colour: bool,
    pub log_output: bool,
    pub log_input: bool,
    pub log_notes: bool,
    pub log_mode: LogMode,
    pub auto_log_file_name: String,
    pub write_world_name_to_log: bool,
    pub log_line_preamble_output: String,
    pub log_line_preamble_input: String,
    pub log_line_preamble_notes: String,
    pub log_line_postamble_output: String,
    pub log_line_postamble_input: String,
    pub log_line_postamble_notes: String,
    pub log_script_errors: bool,

    // Timers
    pub enable_timers: bool,

    // Output
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub indent_paras: u8,
    pub ansi_colours: [RgbColor; 16],
    pub use_default_colours: bool,
    pub display_my_input: bool,
    pub echo_colour: Option<RgbColor>,
    pub echo_background_colour: Option<RgbColor>,
    pub keep_commands_on_same_line: bool,
    pub new_activity_sound: String,
    pub line_information: bool,

    // MUD
    pub use_mxp: UseMxp,
    pub ignore_mxp_colour_changes: bool,
    pub use_custom_link_colour: bool,
    pub hyperlink_colour: RgbColor,
    pub mud_can_change_link_colour: bool,
    pub underline_hyperlinks: bool,
    pub mud_can_remove_underline: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub terminal_identification: String,
    pub disable_compression: bool,
    pub naws: bool,
    pub carriage_return_clears_line: bool,
    pub utf_8: bool,
    pub convert_ga_to_newline: bool,
    pub no_echo_off: bool,
    pub enable_command_stack: bool,
    pub command_stack_character: u8,
    pub mxp_debug_level: MxpDebugLevel,

    // Triggers
    pub enable_triggers: bool,
    pub enable_trigger_sounds: bool,

    // Aliases
    pub enable_aliases: bool,

    // Keypad
    pub numpad_shortcuts: NumpadMapping,
    pub keypad_enable: bool,
    pub hotkey_adds_to_command_history: bool,
    pub echo_hotkey_in_output_window: bool,

    // Scripting
    pub enable_scripts: bool,
    pub world_script: String,
    pub script_reload_option: ScriptRecompile,
    pub note_text_colour: Option<RgbColor>,
    pub note_background_colour: Option<RgbColor>,
    pub script_errors_to_output_window: bool,
    pub error_text_colour: Option<RgbColor>,
    pub error_background_colour: Option<RgbColor>,

    // Hidden
    pub id: Uuid,
    pub plugins: Vec<PathBuf>,
}

impl Default for WorldConfig {
    fn default() -> Self {
        Self::new()
    }
}

impl WorldConfig {
    pub fn new() -> Self {
        Self {
            // Connecting
            name: String::new(),
            site: String::new(),
            port: 4000,
            use_ssl: false,
            use_proxy: false,
            proxy_server: String::new(),
            proxy_port: 1080,
            proxy_username: String::new(),
            proxy_password: String::new(),
            save_world_automatically: false,

            // Login
            player: String::new(),
            password: String::new(),
            connect_method: AutoConnect::None,
            connect_text: String::new(),

            // Logging
            log_file_preamble: String::new(),
            log_file_postamble: String::new(),
            log_format: LogFormat::Text,
            log_in_colour: false,
            log_output: true,
            log_input: true,
            log_notes: true,
            log_mode: LogMode::Append,
            auto_log_file_name: String::new(),
            write_world_name_to_log: false,
            log_line_preamble_output: String::new(),
            log_line_preamble_input: String::new(),
            log_line_preamble_notes: String::new(),
            log_line_postamble_output: String::new(),
            log_line_postamble_input: String::new(),
            log_line_postamble_notes: String::new(),
            log_script_errors: false,

            // Timers
            enable_timers: true,

            // Output
            show_bold: true,
            show_italic: true,
            show_underline: true,
            indent_paras: 0,
            ansi_colours: RgbColor::XTERM_16,
            use_default_colours: false,
            display_my_input: true,
            echo_colour: Some(RgbColor::rgb(128, 128, 128)),
            echo_background_colour: None,
            keep_commands_on_same_line: false,
            new_activity_sound: String::new(),
            line_information: false,

            // MUD
            use_mxp: UseMxp::Command,
            ignore_mxp_colour_changes: false,
            use_custom_link_colour: false,
            hyperlink_colour: RgbColor::rgb(43, 121, 162),
            mud_can_change_link_colour: true,
            underline_hyperlinks: true,
            mud_can_remove_underline: false,
            hyperlink_adds_to_command_history: true,
            echo_hyperlink_in_output_window: true,
            terminal_identification: "mushclient".to_owned(),
            disable_compression: false,
            naws: false,
            carriage_return_clears_line: false,
            utf_8: true,
            convert_ga_to_newline: false,
            no_echo_off: false,
            enable_command_stack: false,
            command_stack_character: b';',
            mxp_debug_level: MxpDebugLevel::None,

            // Triggers
            enable_triggers: true,
            enable_trigger_sounds: true,

            // Aliases
            enable_aliases: true,

            // Keypad
            numpad_shortcuts: NumpadMapping::navigation(),
            keypad_enable: true,
            hotkey_adds_to_command_history: false,
            echo_hotkey_in_output_window: true,

            // Scripting
            enable_scripts: true,
            world_script: String::new(),
            script_reload_option: ScriptRecompile::Confirm,
            note_text_colour: Some(RgbColor::rgb(0, 128, 255)),
            note_background_colour: None,
            script_errors_to_output_window: false,
            error_text_colour: Some(RgbColor::rgb(228, 56, 56)),
            error_background_colour: None,

            // Hidden
            id: Uuid::new_v4(),
            plugins: Vec::new(),
        }
    }

    pub fn escape<'a>(&self, message: &'a str) -> Escaped<Cow<'a, str>> {
        Escaped::borrow(message, self)
    }

    pub fn brackets(&self) -> LogBrackets {
        LogBrackets::from(self)
    }

    pub fn log_path(&self) -> String {
        if self.auto_log_file_name.is_empty() {
            let mut path = format!("{} log.txt", self.name);
            path.retain(|c| !"<>\"|?:#%;/\\".contains(c));
            return path;
        }
        let mut path = String::new();
        Local::now()
            .format(&self.auto_log_file_name)
            .write_to(&mut path)
            .unwrap();
        path
    }

    pub fn world_plugin(&self) -> Plugin {
        let today = Local::now().date_naive();
        Plugin {
            disabled: Cell::new(!self.enable_scripts),
            metadata: PluginMetadata {
                name: format!("World Script: {}", self.name),
                written: today,
                modified: today,
                is_world_plugin: true,
                sequence: -1,
                path: PathBuf::from(&self.world_script),
                ..Default::default()
            },
            ..Default::default()
        }
    }

    pub fn connect_message(&self) -> String {
        let player = &self.player;
        let password = &self.password;
        let text = if self.connect_text.is_empty() {
            String::new()
        } else {
            let mut text = self
                .connect_text
                .replace("%name%", player)
                .replace("%password%", password);
            if !text.ends_with('\n') {
                text.push_str("\r\n");
            }
            text
        };
        match self.connect_method {
            AutoConnect::Diku if password.is_empty() => format!("{player}\r\n{text}"),
            AutoConnect::Diku => format!("{player}\r\n{password}\r\n{text}"),
            AutoConnect::Mush => format!("connect {player} {password}\r\n{text}"),
            AutoConnect::Mxp | AutoConnect::None => text,
        }
    }
}

impl From<&WorldConfig> for TransformerConfig {
    fn from(value: &WorldConfig) -> Self {
        let (player, password) = if value.connect_method == AutoConnect::Mxp {
            (value.player.clone(), value.password.clone())
        } else {
            (String::new(), String::new())
        };
        Self {
            use_mxp: value.use_mxp,
            disable_compression: value.disable_compression,
            terminal_identification: value.terminal_identification.clone(),
            player,
            password,
            convert_ga_to_newline: value.convert_ga_to_newline,
            no_echo_off: value.no_echo_off,
            naws: value.naws,
            disable_utf8: !value.utf_8,
            ignore_mxp_colors: value.ignore_mxp_colour_changes,
            colors: value.ansi_colours.to_vec(),
            ..Default::default()
        }
    }
}
