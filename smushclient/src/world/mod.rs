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
use std::cell::{Cell, Ref, RefMut};
use std::collections::HashSet;
use std::fmt::Write as _;
use std::io::{Read, Write};
use std::path::PathBuf;

use chrono::Local;
use mud_transformer::mxp::RgbColor;
use mud_transformer::{TransformerConfig, UseMxp};
use serde::{Deserialize, Serialize};
use smushclient_plugins::{Alias, CursorVec, Plugin, PluginMetadata, Sender, Timer, Trigger};

use crate::collections::SortOnDrop;
use crate::plugins::{SendIterable, SenderAccessError};

const CURRENT_VERSION: u16 = 3;

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
    pub timers: CursorVec<Timer>,
    pub enable_timers: bool,

    // Output
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub indent_paras: u8,
    pub ansi_colours: [RgbColor; 16],
    pub display_my_input: bool,
    pub echo_colours: ColorPair,
    pub keep_commands_on_same_line: bool,
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
    pub no_echo_off: bool,
    pub enable_command_stack: bool,
    pub command_stack_character: u16,

    // Triggers
    #[serde(serialize_with = "skip_temporary")]
    pub triggers: CursorVec<Trigger>,
    pub enable_triggers: bool,

    // Aliases
    #[serde(serialize_with = "skip_temporary")]
    pub aliases: CursorVec<Alias>,
    pub enable_aliases: bool,

    // Keypad
    pub numpad_shortcuts: NumpadMapping,
    pub numpad_enable: bool,
    pub hotkey_adds_to_command_history: bool,
    pub echo_hotkey_in_output_window: bool,

    // Scripting
    pub enable_scripts: bool,
    pub world_script: Option<String>,
    pub script_reload_option: ScriptRecompile,
    pub note_colours: ColorPair,
    pub error_colours: ColorPair,

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
            timers: CursorVec::new(),
            enable_timers: true,

            // Output
            show_bold: true,
            show_italic: true,
            show_underline: true,
            indent_paras: 0,
            ansi_colours: *RgbColor::XTERM_16,
            display_my_input: true,
            echo_colours: ColorPair::foreground(RgbColor::rgb(128, 128, 128)),
            keep_commands_on_same_line: false,
            new_activity_sound: None,

            // MUD
            use_mxp: UseMxp::Command,
            ignore_mxp_colour_changes: false,
            use_custom_link_colour: false,
            hyperlink_colour: RgbColor::rgb(43, 121, 162),
            mud_can_change_link_colour: true,
            underline_hyperlinks: true,
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
            command_stack_character: u16::from(b';'),

            // Triggers
            triggers: CursorVec::new(),
            enable_triggers: true,

            // Aliases
            aliases: CursorVec::new(),
            enable_aliases: true,

            // Keypad
            numpad_shortcuts: NumpadMapping::navigation(),
            numpad_enable: true,
            hotkey_adds_to_command_history: false,
            echo_hotkey_in_output_window: true,

            // Scripting
            enable_scripts: true,
            world_script: None,
            script_reload_option: ScriptRecompile::Confirm,
            note_colours: ColorPair::foreground(RgbColor::rgb(0, 128, 255)),
            error_colours: ColorPair::foreground(RgbColor::rgb(127, 0, 0)),

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
        let mut version_buf = [0; 2];
        reader.read_exact(&mut version_buf)?;
        let mut buf = Vec::new();
        let version = u16::from_be_bytes(version_buf);
        if version > 2 {
            reader.read_to_end(&mut buf)?;
        }
        match version {
            1 => versions::V1::migrate(&mut reader),
            2 => versions::V2::migrate(&mut reader),
            3 => postcard::from_bytes(&buf).map_err(Into::into),
            _ => Err(PersistError::Invalid)?,
        }
    }

    pub fn palette(&self) -> [RgbColor; 164] {
        let mut palette = [RgbColor::BLACK; 164];
        palette[0..16].copy_from_slice(&self.ansi_colours);
        for (slot, (_name, color)) in palette[17..].iter_mut().zip(RgbColor::iter_named()) {
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

    pub fn import_senders<T: SendIterable>(
        &self,
        imported: &mut Vec<T>,
    ) -> RefMut<'_, SortOnDrop<T>> {
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
        RefMut::map(senders, |senders| {
            SortOnDrop::borrow_mut(&mut senders[senders_len..])
        })
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
            colors: value.ansi_colours.to_vec(),
            ..Default::default()
        }
    }
}
