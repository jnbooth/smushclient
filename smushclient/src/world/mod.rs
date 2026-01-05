mod error;
pub use error::PersistError;

mod escaping;
pub use escaping::{Escaped, EscapedBrackets, LogBrackets};

mod sender_map;
pub use sender_map::SenderMap;

mod types;
pub use types::*;

mod versions;
use std::borrow::Cow;
use std::cell::{Cell, Ref};
use std::collections::HashSet;
use std::fmt::Write as _;
use std::io::{Read, Write};
use std::path::PathBuf;

use chrono::Local;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{TransformerConfig, UseMxp};
use serde::{Deserialize, Serialize};
use smushclient_plugins::{Alias, CursorVec, Plugin, PluginMetadata, Sender, Timer, Trigger};
use versions::Migrate;

use crate::collections::SortOnDrop;
use crate::plugins::{SendIterable, SenderAccessError};

const CURRENT_VERSION: u16 = 2;

fn skip_temporary<S, T>(vec: &CursorVec<T>, serializer: S) -> Result<S::Ok, S::Error>
where
    S: serde::Serializer,
    T: serde::Serialize + AsRef<Sender> + Ord,
{
    let items = vec.borrow();
    // must collect in a vec because serialization needs to know the size ahead of time
    let filtered: Vec<&T> = items.iter().filter(|x| !x.as_ref().temporary).collect();
    serializer.collect_seq(filtered)
}

#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct World {
    // Connecting
    pub name: String,
    pub site: String,
    pub port: u16,
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
    pub auto_log_file_name: Option<String>,
    pub write_world_name_to_log: bool,
    pub log_line_preamble_output: String,
    pub log_line_preamble_input: String,
    pub log_line_preamble_notes: String,
    pub log_line_postamble_output: String,
    pub log_line_postamble_input: String,
    pub log_line_postamble_notes: String,
    pub log_script_errors: bool,

    // Timers
    #[serde(serialize_with = "skip_temporary")]
    pub timers: CursorVec<Timer>,
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
    pub new_activity_sound: Option<String>,
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
    pub mxp_debug_level: MXPDebugLevel,

    // Triggers
    #[serde(serialize_with = "skip_temporary")]
    pub triggers: CursorVec<Trigger>,
    pub enable_triggers: bool,
    pub enable_trigger_sounds: bool,

    // Aliases
    #[serde(serialize_with = "skip_temporary")]
    pub aliases: CursorVec<Alias>,
    pub enable_aliases: bool,

    // Keypad
    pub numpad_shortcuts: NumpadMapping,
    pub keypad_enable: bool,
    pub hotkey_adds_to_command_history: bool,
    pub echo_hotkey_in_output_window: bool,

    // Scripting
    pub enable_scripts: bool,
    pub world_script: Option<String>,
    pub script_reload_option: ScriptRecompile,
    pub note_text_colour: Option<RgbColor>,
    pub note_background_colour: Option<RgbColor>,
    pub script_errors_to_output_window: bool,
    pub error_text_colour: Option<RgbColor>,
    pub error_background_colour: Option<RgbColor>,

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
            // Connecting
            name: String::new(),
            site: String::new(),
            port: 4000,
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
            auto_log_file_name: None,
            write_world_name_to_log: false,
            log_line_preamble_output: String::new(),
            log_line_preamble_input: String::new(),
            log_line_preamble_notes: String::new(),
            log_line_postamble_output: String::new(),
            log_line_postamble_input: String::new(),
            log_line_postamble_notes: String::new(),
            log_script_errors: false,

            // Timers
            timers: CursorVec::new(),
            enable_timers: true,

            // Output
            show_bold: true,
            show_italic: true,
            show_underline: true,
            indent_paras: 0,
            ansi_colours: *RgbColor::XTERM_16,
            use_default_colours: false,
            display_my_input: true,
            echo_colour: Some(RgbColor::rgb(128, 128, 128)),
            echo_background_colour: None,
            keep_commands_on_same_line: false,
            new_activity_sound: None,
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
            mxp_debug_level: MXPDebugLevel::None,

            // Triggers
            triggers: CursorVec::new(),
            enable_triggers: true,
            enable_trigger_sounds: true,

            // Aliases
            aliases: CursorVec::new(),
            enable_aliases: true,

            // Keypad
            numpad_shortcuts: NumpadMapping::navigation(),
            keypad_enable: true,
            hotkey_adds_to_command_history: false,
            echo_hotkey_in_output_window: true,

            // Scripting
            enable_scripts: true,
            world_script: None,
            script_reload_option: ScriptRecompile::Confirm,
            note_text_colour: Some(RgbColor::rgb(0, 128, 255)),
            note_background_colour: None,
            script_errors_to_output_window: false,
            error_text_colour: Some(RgbColor::rgb(127, 0, 0)),
            error_background_colour: None,

            // Hidden
            plugins: Vec::new(),
        }
    }

    pub fn escape<'a>(&self, message: &'a str) -> Escaped<Cow<'a, str>> {
        Escaped::borrow(message, self)
    }

    pub fn brackets(&self) -> LogBrackets {
        LogBrackets::from(self)
    }

    pub fn world_plugin(&self) -> Plugin {
        let today = Local::now().date_naive();
        let path = match &self.world_script {
            Some(world_script) => PathBuf::from(&world_script),
            None => PathBuf::new(),
        };
        Plugin {
            disabled: Cell::new(!self.enable_scripts),
            metadata: PluginMetadata {
                name: format!("World Script: {}", self.name),
                written: today,
                modified: today,
                is_world_plugin: true,
                sequence: -1,
                path,
                ..Default::default()
            },
            ..Default::default()
        }
    }

    pub fn add_sender<T: SendIterable>(&self, sender: T) -> Result<Ref<'_, T>, SenderAccessError> {
        let senders = T::from_world(self);
        sender.assert_unique_label(senders)?;
        Ok(senders.insert(sender))
    }

    pub fn replace_sender<T: SendIterable>(
        &self,
        index: usize,
        sender: T,
    ) -> Result<(usize, Ref<'_, T>), SenderAccessError> {
        let senders = T::from_world(self);
        if *senders.get(index).ok_or(SenderAccessError::NotFound)? == sender {
            return Err(SenderAccessError::Unchanged);
        }
        if let Err(pos) = sender.assert_unique_label(senders)
            && pos != index
        {
            return Err(pos.into());
        }
        Ok(senders.replace(index, sender))
    }

    pub fn remove_temporary(&self) {
        self.aliases.retain(|sender| !sender.temporary);
        self.timers.retain(|sender| !sender.temporary);
        self.triggers.retain(|sender| !sender.temporary);
    }

    pub fn save<W: Write>(&self, mut writer: W) -> Result<(), PersistError> {
        writer.write_all(&CURRENT_VERSION.to_be_bytes())?;
        postcard::to_io(self, writer)?;
        Ok(())
    }

    pub fn load<R: Read>(mut reader: R) -> Result<Self, PersistError> {
        let mut buf = Vec::new();
        reader.read_to_end(&mut buf)?;
        let (version, bytes) = buf.split_at_checked(2).ok_or(PersistError::Invalid)?;
        let version = u16::from_be_bytes(version.try_into()?);
        match version {
            1 | 3 => versions::V1::migrate(bytes),
            2 => postcard::from_bytes(bytes).map_err(Into::into),
            _ => Err(PersistError::Invalid)?,
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

    pub fn import_senders<T: SendIterable>(&self, imported: &mut Vec<T>) -> SortOnDrop<'_, T> {
        let mut senders = T::from_world(self).borrow_mut();
        let senders_len = senders.len();
        let need_relabeling = !imported
            .iter()
            .all(|sender| sender.as_ref().label.is_empty());
        senders.append(imported);
        if need_relabeling {
            let mut labels = HashSet::new();
            for sender in senders.iter_mut() {
                let sender = sender.as_mut();
                if !labels.contains(&sender.label) {
                    labels.insert(&sender.label);
                    continue;
                }
                let len = sender.label.len();
                for i in 0.. {
                    write!(sender.label, "{i}").unwrap();
                    if !labels.contains(&sender.label) {
                        labels.insert(&sender.label);
                        break;
                    }
                    sender.label.truncate(len);
                }
            }
        }
        SortOnDrop::new(senders, senders_len)
    }
}

impl From<&World> for TransformerConfig {
    fn from(value: &World) -> Self {
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
