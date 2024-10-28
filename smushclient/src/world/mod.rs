mod error;
pub use error::PersistError;

mod types;
pub use types::*;

mod versions;
use versions::WorldVersion;

use std::collections::HashMap;
use std::io::{Read, Write};
use std::path::PathBuf;

use chrono::Utc;
use mud_transformer::{TransformerConfig, UseMxp};
use serde::{Deserialize, Serialize};
use smushclient_plugins::{Alias, Plugin, PluginMetadata, Sender, Timer, Trigger};

use mud_transformer::mxp::RgbColor;

const CURRENT_VERSION: u8 = 3;

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

    // Notes
    pub notes: String,

    // Output
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub new_activity_sound: Option<String>,

    pub disable_compression: bool,
    pub indent_paras: u8,
    pub naws: bool,
    pub carriage_return_clears_line: bool,
    pub utf_8: bool,
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
    #[serde(serialize_with = "skip_temporary")]
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,

    // Commands
    pub display_my_input: bool,
    pub keep_commands_on_same_line: bool,
    pub no_echo_off: bool,
    pub echo_colors: ColorPair,
    pub enable_speed_walk: bool,
    pub speed_walk_prefix: String,
    pub speed_walk_filler: String,
    pub command_queue_delay: f64,
    pub enable_command_stack: bool,
    pub command_stack_character: u16,
    pub enable_spam_prevention: bool,
    pub spam_line_count: usize,
    pub spam_message: String,

    // Aliases
    #[serde(serialize_with = "skip_temporary")]
    pub aliases: Vec<Alias>,
    pub enable_aliases: bool,

    // Keypad
    pub keypad_enable: bool,
    pub keypad_shortcuts: HashMap<String, String>,

    // Paste
    pub paste_line_preamble: String,
    pub paste_line_postamble: String,
    pub paste_delay: u32,
    pub paste_delay_per_lines: u32,
    pub paste_echo: bool,

    // Scripts
    pub world_script: String,
    pub enable_scripts: bool,
    pub script_reload_option: ScriptRecompile,
    pub error_colour: RgbColor,
    pub note_text_colour: RgbColor,

    // Hidden
    pub plugins: Vec<PathBuf>,
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
            use_proxy: false,
            proxy_server: String::new(),
            proxy_port: 1080,
            proxy_username: String::new(),
            proxy_password: String::new(),
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

            // Notes
            notes: String::new(),

            // Output
            show_bold: true,
            show_italic: true,
            show_underline: true,
            new_activity_sound: None,

            disable_compression: false,
            indent_paras: 0,
            naws: false,
            carriage_return_clears_line: false,
            utf_8: true,
            convert_ga_to_newline: false,
            terminal_identification: "mushclient".to_owned(),

            // MXP
            use_mxp: UseMxp::Command,
            hyperlink_colour: RgbColor::rgb(43, 121, 162),
            use_custom_link_colour: false,
            mud_can_change_link_colour: true,
            underline_hyperlinks: true,
            mud_can_remove_underline: false,
            hyperlink_adds_to_command_history: true,
            echo_hyperlink_in_output_window: true,
            ignore_mxp_colour_changes: false,
            send_mxp_afk_response: true,

            // ANSI Color
            use_default_colours: true,
            ansi_colors: *RgbColor::XTERM_16,
            error_colour: RgbColor::rgb(127, 0, 0),

            // Triggers
            triggers: Vec::new(),
            enable_triggers: true,

            // Commands
            display_my_input: true,
            keep_commands_on_same_line: false,
            echo_colors: ColorPair::foreground(RgbColor::rgb(128, 128, 128)),
            enable_speed_walk: false,
            speed_walk_prefix: "#".to_owned(),
            speed_walk_filler: "a".to_owned(),
            command_queue_delay: 0.0,
            enable_command_stack: false,
            command_stack_character: u16::from(b';'),
            enable_spam_prevention: false,
            spam_line_count: 20,
            spam_message: "look".to_owned(),

            no_echo_off: false,

            // Aliases
            aliases: Vec::new(),
            enable_aliases: true,

            // Keypad
            keypad_enable: true,
            keypad_shortcuts: HashMap::new(),

            // Paste
            paste_line_preamble: String::new(),
            paste_line_postamble: String::new(),
            paste_delay: 0,
            paste_delay_per_lines: 1,
            paste_echo: false,

            // Scripts
            world_script: String::new(),
            enable_scripts: true,
            script_reload_option: ScriptRecompile::Confirm,
            note_text_colour: RgbColor::rgb(0, 128, 255),

            // Hidden
            plugins: Vec::new(),
        }
    }

    pub fn world_plugin(&self) -> Plugin {
        let today = Utc::now().date_naive();
        let metadata = PluginMetadata {
            name: format!("World Script: {}", self.name),
            written: today,
            modified: today,
            is_world_plugin: true,
            sequence: -1,
            ..Default::default()
        };
        Plugin {
            metadata,
            disabled: false,
            triggers: Vec::new(),
            aliases: Vec::new(),
            timers: Vec::new(),
            script: self.world_script.clone(),
        }
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
            1 => versions::V1::migrate(&mut reader),
            2 => versions::V2::migrate(&mut reader),
            3 => bincode::deserialize_from(reader).map_err(Into::into),
            _ => Err(PersistError::Invalid)?,
        }
    }

    pub fn palette(&self) -> [RgbColor; 166] {
        let mut palette = [RgbColor::BLACK; 166];
        palette[0] = self.note_text_colour;
        palette[1] = self.error_colour;
        palette[2..18].copy_from_slice(&self.ansi_colors);
        for (slot, (_name, color)) in palette[19..].iter_mut().zip(RgbColor::iter_named()) {
            *slot = color;
        }
        palette
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
            Some(AutoConnect::Diku) if password.is_empty() => format!("{player}\r\n{text}"),
            Some(AutoConnect::Diku) => format!("{player}\r\n{password}\r\n{text}"),
            Some(AutoConnect::Mush) => format!("connect {player} {password}\r\n{text}"),
            Some(AutoConnect::Mxp) | None => text,
        }
    }
}

impl From<&World> for TransformerConfig {
    fn from(value: &World) -> Self {
        let (player, password) = if value.connect_method == Some(AutoConnect::Mxp) {
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
            colors: value.ansi_colors.to_vec(),
            ..Default::default()
        }
    }
}
