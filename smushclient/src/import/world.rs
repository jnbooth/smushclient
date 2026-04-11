use std::borrow::Cow;
use std::collections::HashMap;
use std::io::BufRead;
use std::time::Duration;

use base64::Engine;
use mud_transformer::UseMxp;
use mud_transformer::opt::mxp::RgbColor;
use serde::Deserialize;
use smushclient_plugins::xml::{XmlAlias, XmlTimer, XmlTrigger, XmlVec};
use smushclient_plugins::{ImportError, RegexError};
use uuid::Uuid;

use super::color::{Colours, ColoursXml};
use super::xml_enum::XmlEnum;
use crate::client::{PluginVariableMap, XmlVariable};
use crate::world::{
    AutoConnect, LogFormat, LogMode, MxpDebugLevel, ScriptRecompile, World, WorldConfig, XmlKey,
};

mod bool_serde {
    pub use serde_this_or_that::as_bool as deserialize;
}

mod bool_opt_serde {
    pub use serde_this_or_that::as_opt_bool as deserialize;
}

#[derive(Debug, Deserialize)]
#[serde(rename = "include")]
pub(super) struct XmlInclude {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@plugin", with = "bool_serde", default)]
    pub plugin: bool,
}

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "world", default)]
pub(super) struct XmlWorld {
    // NUMERIC OPTIONS
    #[serde(rename = "@carriage_return_clears_line", with = "bool_serde")]
    pub carriage_return_clears_line: bool,
    #[serde(rename = "@connect_method", with = "XmlEnum")]
    pub connect_method: AutoConnect,
    #[serde(rename = "@convert_ga_to_newline", with = "bool_serde")]
    pub convert_ga_to_newline: bool,
    #[serde(rename = "@disable_compression", with = "bool_serde")]
    pub disable_compression: bool,
    #[serde(rename = "@display_my_input", with = "bool_serde")]
    pub display_my_input: bool,
    #[serde(rename = "@echo_colour")]
    pub echo_colour: Option<usize>,
    #[serde(rename = "@echo_hyperlink_in_output_window", with = "bool_serde")]
    pub echo_hyperlink_in_output_window: bool,
    #[serde(rename = "@enable_aliases", with = "bool_serde")]
    pub enable_aliases: bool,
    #[serde(rename = "@enable_command_stack", with = "bool_serde")]
    pub enable_command_stack: bool,
    #[serde(rename = "@enable_scripts", with = "bool_serde")]
    pub enable_scripts: bool,
    #[serde(rename = "@enable_speed_walk", with = "bool_serde")]
    pub enable_speed_walk: bool,
    #[serde(rename = "@enable_timers", with = "bool_serde")]
    pub enable_timers: bool,
    #[serde(rename = "@enable_triggers", with = "bool_serde")]
    pub enable_triggers: bool,
    #[serde(rename = "@enable_trigger_sounds", with = "bool_serde")]
    pub enable_trigger_sounds: bool,
    #[serde(rename = "@hyperlink_adds_to_command_history", with = "bool_serde")]
    pub hyperlink_adds_to_command_history: bool,
    #[serde(rename = "@hyperlink_colour")]
    pub hyperlink_colour: Option<RgbColor>,
    #[serde(rename = "@ignore_mxp_colour_changes", with = "bool_serde")]
    pub ignore_mxp_colour_changes: bool,
    #[serde(rename = "@indent_paras", with = "bool_serde")]
    pub indent_paras: bool,
    #[serde(rename = "@keep_commands_on_same_line", with = "bool_serde")]
    pub keep_commands_on_same_line: bool,
    #[serde(rename = "@keypad_enable", with = "bool_serde")]
    pub keypad_enable: bool,
    #[serde(rename = "@line_information", with = "bool_serde")]
    pub line_information: bool,
    #[serde(rename = "@log_html", with = "bool_serde")]
    pub log_html: bool,
    #[serde(rename = "@log_input", with = "bool_serde")]
    pub log_input: bool,
    #[serde(rename = "@log_in_colour", with = "bool_serde")]
    pub log_in_colour: bool,
    #[serde(rename = "@log_notes", with = "bool_serde")]
    pub log_notes: bool,
    #[serde(rename = "@log_output", with = "bool_serde")]
    pub log_output: bool,
    #[serde(rename = "@log_raw", with = "bool_serde")]
    pub log_raw: bool,
    #[serde(rename = "@log_script_errors", with = "bool_serde")]
    pub log_script_errors: bool,
    #[serde(rename = "@mud_can_change_link_colour", with = "bool_serde")]
    pub mud_can_change_link_colour: bool,
    #[serde(rename = "@mud_can_remove_underline", with = "bool_serde")]
    pub mud_can_remove_underline: bool,
    #[serde(rename = "@mxp_debug_level", with = "XmlEnum")]
    pub mxp_debug_level: MxpDebugLevel,
    #[serde(rename = "@naws", with = "bool_serde")]
    pub naws: bool,
    #[serde(rename = "@note_text_colour")]
    pub note_text_colour: Option<RgbColor>,
    #[serde(rename = "@no_echo_off", with = "bool_serde")]
    pub no_echo_off: bool,
    #[serde(rename = "@play_sounds_in_background", with = "bool_serde")]
    pub play_sounds_in_background: bool,
    #[serde(rename = "@port")]
    pub port: u16,
    #[serde(rename = "@proxy_port")]
    pub proxy_port: u16,
    #[serde(rename = "@proxy_type")]
    pub proxy_type: u8,
    #[serde(rename = "@save_world_automatically", with = "bool_serde")]
    pub save_world_automatically: bool,
    #[serde(rename = "@script_reload_option", with = "XmlEnum")]
    pub script_reload_option: ScriptRecompile,
    #[serde(rename = "@script_errors_to_output_window", with = "bool_serde")]
    pub script_errors_to_output_window: bool,
    #[serde(rename = "@show_bold", with = "bool_serde")]
    pub show_bold: bool,
    #[serde(rename = "@show_italic", with = "bool_serde")]
    pub show_italic: bool,
    #[serde(rename = "@show_underline", with = "bool_serde")]
    pub show_underline: bool,
    #[serde(rename = "@speed_walk_delay")]
    pub speed_walk_delay: u16,
    #[serde(rename = "@underline_hyperlinks", with = "bool_serde")]
    pub underline_hyperlinks: bool,
    #[serde(rename = "@use_custom_link_colour", with = "bool_serde")]
    pub use_custom_link_colour: bool,
    #[serde(rename = "@use_default_colours", with = "bool_serde")]
    pub use_default_colours: bool,
    #[serde(rename = "@use_mxp", with = "XmlEnum")]
    pub use_mxp: UseMxp,
    #[serde(rename = "@utf_8", with = "bool_serde")]
    pub utf_8: bool,
    #[serde(rename = "@write_world_name_to_log", with = "bool_serde")]
    pub write_world_name_to_log: bool,

    // ALPHA OPTIONS
    #[serde(rename = "@auto_log_file_name")]
    pub auto_log_file_name: String,
    #[serde(rename = "@command_stack_character")]
    pub command_stack_character: String,
    pub connect_text: String,
    pub log_file_postamble: String,
    pub log_file_preamble: String,
    #[serde(rename = "@log_line_postamble_input")]
    pub log_line_postamble_input: String,
    #[serde(rename = "@log_line_postamble_notes")]
    pub log_line_postamble_notes: String,
    #[serde(rename = "@log_line_postamble_output")]
    pub log_line_postamble_output: String,
    #[serde(rename = "@log_line_preamble_input")]
    pub log_line_preamble_input: String,
    #[serde(rename = "@log_line_preamble_notes")]
    pub log_line_preamble_notes: String,
    #[serde(rename = "@log_line_preamble_output")]
    pub log_line_preamble_output: String,
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@new_activity_sound")]
    pub new_activity_sound: String,
    #[serde(rename = "@password")]
    pub password: String,
    #[serde(rename = "@password_base64", with = "bool_serde")]
    pub password_base64: bool,
    #[serde(rename = "@player")]
    pub player: String,
    #[serde(rename = "@proxy_server")]
    pub proxy_server: String,
    #[serde(rename = "@proxy_username")]
    pub proxy_username: String,
    #[serde(rename = "@proxy_password")]
    pub proxy_password: String,
    #[serde(rename = "@proxy_password_base64", with = "bool_serde")]
    pub proxy_password_base64: bool,
    #[serde(rename = "@script_filename")]
    pub script_filename: String,
    #[serde(rename = "@site")]
    pub site: String,
    #[serde(rename = "@speed_walk_filler")]
    pub speed_walk_filler: String,
    #[serde(rename = "@speed_walk_prefix")]
    pub speed_walk_prefix: String,
    #[serde(rename = "@terminal_identification")]
    pub terminal_identification: String,
}

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "muclient", default)]
pub(super) struct MuClient<'a, W = XmlWorld> {
    pub world: W,
    #[serde(borrow)]
    pub triggers: XmlVec<XmlTrigger<'a>>,
    #[serde(borrow)]
    pub aliases: XmlVec<XmlAlias<'a>>,
    #[serde(borrow)]
    pub timers: XmlVec<XmlTimer<'a>>,
    // pub macros: XmlVec<Macro>,
    pub variables: XmlVec<XmlVariable<'a>>,
    pub colours: ColoursXml,
    pub keypad: XmlVec<XmlKey<'a>>,
    // pub printing: Printing,
    pub include: Vec<XmlInclude>,
}

#[derive(Debug, Deserialize)]
#[serde(try_from = "MuClient")]
pub(crate) struct ImportedWorld {
    pub world: World<'static>,
    pub variables: PluginVariableMap,
}

impl ImportedWorld {
    pub fn from_xml<R: BufRead>(reader: R) -> Result<Self, ImportError> {
        let mut deserializer = quick_xml::de::Deserializer::from_reader(reader);
        Ok(MuClient::deserialize(&mut deserializer)?.try_into()?)
    }
}

fn decode_password(password: String, encoded: bool) -> String {
    if !encoded {
        return password;
    }
    let Ok(decoded) = base64::prelude::BASE64_STANDARD.decode(&password) else {
        return password;
    };
    String::from_utf8(decoded).unwrap_or(password)
}

fn first_char(s: &str) -> Option<u8> {
    match s.as_bytes() {
        &[first, ..] => Some(first),
        _ => None,
    }
}

impl TryFrom<MuClient<'_>> for ImportedWorld {
    type Error = RegexError;

    fn try_from(value: MuClient<'_>) -> Result<Self, Self::Error> {
        let MuClient {
            world,
            mut triggers,
            aliases,
            timers,
            variables,
            colours,
            keypad,
            include,
        } = value;

        let colours = Colours::from(colours);

        let XmlWorld {
            carriage_return_clears_line,
            connect_method,
            convert_ga_to_newline,
            disable_compression,
            display_my_input,
            echo_colour,
            echo_hyperlink_in_output_window,
            enable_aliases,
            enable_command_stack,
            enable_scripts,
            enable_speed_walk,
            enable_timers,
            enable_triggers,
            enable_trigger_sounds,
            hyperlink_adds_to_command_history,
            hyperlink_colour,
            ignore_mxp_colour_changes,
            indent_paras,
            keep_commands_on_same_line,
            keypad_enable,
            line_information,
            log_html,
            log_input,
            log_in_colour,
            log_notes,
            log_output,
            log_raw,
            log_script_errors,
            mud_can_change_link_colour,
            mud_can_remove_underline,
            mxp_debug_level,
            naws,
            note_text_colour,
            no_echo_off,
            play_sounds_in_background: _,
            port,
            proxy_port,
            proxy_type,
            save_world_automatically,
            script_reload_option,
            script_errors_to_output_window,
            show_bold,
            show_italic,
            show_underline,
            speed_walk_delay,
            underline_hyperlinks,
            use_custom_link_colour,
            use_default_colours,
            use_mxp,
            utf_8,
            write_world_name_to_log,
            auto_log_file_name,
            command_stack_character,
            connect_text,
            log_file_postamble,
            log_file_preamble,
            log_line_postamble_input,
            log_line_postamble_notes,
            log_line_postamble_output,
            log_line_preamble_input,
            log_line_preamble_notes,
            log_line_preamble_output,
            name,
            new_activity_sound,
            password,
            password_base64,
            player,
            proxy_server,
            proxy_username,
            proxy_password,
            proxy_password_base64,
            script_filename,
            site,
            speed_walk_filler,
            speed_walk_prefix,
            terminal_identification,
        } = world;

        let config = WorldConfig {
            name,
            site,
            port,
            use_ssl: false,
            use_proxy: proxy_type != 0,
            proxy_server,
            proxy_port,
            proxy_username,
            proxy_password: decode_password(proxy_password, proxy_password_base64),
            save_world_automatically,
            player,
            password: decode_password(password, password_base64),
            connect_method,
            connect_text,
            log_file_preamble,
            log_file_postamble,
            log_format: if log_html {
                LogFormat::Html
            } else if log_raw {
                LogFormat::Raw
            } else {
                LogFormat::Text
            },
            log_in_colour,
            log_output,
            log_input,
            log_notes,
            log_mode: LogMode::Append,
            auto_log_file_name,
            write_world_name_to_log,
            log_line_preamble_output,
            log_line_preamble_input,
            log_line_preamble_notes,
            log_line_postamble_output,
            log_line_postamble_input,
            log_line_postamble_notes,
            log_script_errors,
            enable_timers,
            show_bold,
            show_italic,
            show_underline,
            indent_paras: indent_paras.into(),
            ansi_colours: colours.ansi,
            use_default_colours,
            new_activity_sound,
            line_information,
            colour_map: HashMap::new(),
            use_mxp,
            ignore_mxp_colour_changes,
            use_custom_link_colour,
            hyperlink_colour,
            mud_can_change_link_colour,
            underline_hyperlinks,
            mud_can_remove_underline,
            hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window,
            terminal_identification,
            disable_compression,
            naws,
            carriage_return_clears_line,
            utf_8,
            convert_ga_to_newline,
            no_echo_off,
            mxp_debug_level,
            display_my_input,
            echo_colour: echo_colour.and_then(|colour| Some(colours.custom.get(colour)?.text)),
            echo_background_colour: None,
            keep_commands_on_same_line,
            enable_speed_walk,
            speed_walk_prefix: first_char(&speed_walk_prefix).unwrap_or(b'#'),
            speed_walk_delay: Duration::from_millis(speed_walk_delay.into()),
            speed_walk_filler,
            enable_command_stack,
            command_stack_character: first_char(&command_stack_character).unwrap_or(b';'),
            command_stack_delay: false,
            enable_triggers,
            enable_trigger_sounds,
            enable_aliases,
            numpad_shortcuts: keypad.into(),
            keypad_enable,
            hotkey_adds_to_command_history: false,
            echo_hotkey_in_output_window: true,
            enable_scripts,
            world_script: script_filename,
            script_reload_option,
            note_text_colour,
            note_background_colour: None,
            script_errors_to_output_window,
            error_text_colour: Some(RgbColor::rgb(127, 0, 0)),
            error_background_colour: None,
            id: Uuid::new_v4(),
            plugins: include
                .into_iter()
                .filter(|include| include.plugin)
                .map(|include| include.name.into())
                .collect(),
        };

        for trigger in &mut triggers.elements {
            colours.custom.apply_to_trigger(trigger);
        }

        Ok(Self {
            world: World {
                config: Cow::Owned(config),
                timers: Cow::Owned(timers.try_into()?),
                aliases: Cow::Owned(aliases.try_into()?),
                triggers: Cow::Owned(triggers.try_into()?),
            },
            variables: variables.into(),
        })
    }
}

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "world", default)]
pub(crate) struct XmlWorldPartial {
    // NUMERIC OPTIONS
    #[serde(rename = "@carriage_return_clears_line", with = "bool_opt_serde")]
    pub carriage_return_clears_line: Option<bool>,
    #[serde(
        rename = "@connect_method",
        deserialize_with = "XmlEnum::deserialize_opt"
    )]
    pub connect_method: Option<AutoConnect>,
    #[serde(rename = "@convert_ga_to_newline", with = "bool_opt_serde")]
    pub convert_ga_to_newline: Option<bool>,
    #[serde(rename = "@disable_compression", with = "bool_opt_serde")]
    pub disable_compression: Option<bool>,
    #[serde(rename = "@display_my_input", with = "bool_opt_serde")]
    pub display_my_input: Option<bool>,
    #[serde(rename = "@echo_colour")]
    pub echo_colour: Option<usize>,
    #[serde(rename = "@echo_hyperlink_in_output_window", with = "bool_opt_serde")]
    pub echo_hyperlink_in_output_window: Option<bool>,
    #[serde(rename = "@enable_aliases", with = "bool_opt_serde")]
    pub enable_aliases: Option<bool>,
    #[serde(rename = "@enable_command_stack", with = "bool_opt_serde")]
    pub enable_command_stack: Option<bool>,
    #[serde(rename = "@enable_scripts", with = "bool_opt_serde")]
    pub enable_scripts: Option<bool>,
    #[serde(rename = "@enable_speed_walk", with = "bool_opt_serde")]
    pub enable_speed_walk: Option<bool>,
    #[serde(rename = "@enable_timers", with = "bool_opt_serde")]
    pub enable_timers: Option<bool>,
    #[serde(rename = "@enable_triggers", with = "bool_opt_serde")]
    pub enable_triggers: Option<bool>,
    #[serde(rename = "@enable_trigger_sounds", with = "bool_opt_serde")]
    pub enable_trigger_sounds: Option<bool>,
    #[serde(rename = "@hyperlink_adds_to_command_history", with = "bool_opt_serde")]
    pub hyperlink_adds_to_command_history: Option<bool>,
    #[serde(rename = "@hyperlink_colour")]
    pub hyperlink_colour: Option<RgbColor>,
    #[serde(rename = "@ignore_mxp_colour_changes", with = "bool_opt_serde")]
    pub ignore_mxp_colour_changes: Option<bool>,
    #[serde(rename = "@indent_paras", with = "bool_opt_serde")]
    pub indent_paras: Option<bool>,
    #[serde(rename = "@keep_commands_on_same_line", with = "bool_opt_serde")]
    pub keep_commands_on_same_line: Option<bool>,
    #[serde(rename = "@keypad_enable", with = "bool_opt_serde")]
    pub keypad_enable: Option<bool>,
    #[serde(rename = "@line_information", with = "bool_opt_serde")]
    pub line_information: Option<bool>,
    #[serde(rename = "@log_html", with = "bool_opt_serde")]
    pub log_html: Option<bool>,
    #[serde(rename = "@log_input", with = "bool_opt_serde")]
    pub log_input: Option<bool>,
    #[serde(rename = "@log_in_colour", with = "bool_opt_serde")]
    pub log_in_colour: Option<bool>,
    #[serde(rename = "@log_notes", with = "bool_opt_serde")]
    pub log_notes: Option<bool>,
    #[serde(rename = "@log_output", with = "bool_opt_serde")]
    pub log_output: Option<bool>,
    #[serde(rename = "@log_raw", with = "bool_opt_serde")]
    pub log_raw: Option<bool>,
    #[serde(rename = "@log_script_errors", with = "bool_opt_serde")]
    pub log_script_errors: Option<bool>,
    #[serde(rename = "@mud_can_change_link_colour", with = "bool_opt_serde")]
    pub mud_can_change_link_colour: Option<bool>,
    #[serde(rename = "@mud_can_remove_underline", with = "bool_opt_serde")]
    pub mud_can_remove_underline: Option<bool>,
    #[serde(
        rename = "@mxp_debug_level",
        deserialize_with = "XmlEnum::deserialize_opt"
    )]
    pub mxp_debug_level: Option<MxpDebugLevel>,
    #[serde(rename = "@naws", with = "bool_opt_serde")]
    pub naws: Option<bool>,
    #[serde(rename = "@note_text_colour")]
    pub note_text_colour: Option<RgbColor>,
    #[serde(rename = "@no_echo_off", with = "bool_opt_serde")]
    pub no_echo_off: Option<bool>,
    #[serde(rename = "@play_sounds_in_background", with = "bool_opt_serde")]
    pub play_sounds_in_background: Option<bool>,
    #[serde(rename = "@port")]
    pub port: Option<u16>,
    #[serde(rename = "@proxy_port")]
    pub proxy_port: Option<u16>,
    #[serde(rename = "@proxy_type")]
    pub proxy_type: Option<u8>,
    #[serde(rename = "@save_world_automatically", with = "bool_opt_serde")]
    pub save_world_automatically: Option<bool>,
    #[serde(
        rename = "@script_reload_option",
        deserialize_with = "XmlEnum::deserialize_opt"
    )]
    pub script_reload_option: Option<ScriptRecompile>,
    #[serde(rename = "@script_errors_to_output_window", with = "bool_opt_serde")]
    pub script_errors_to_output_window: Option<bool>,
    #[serde(rename = "@show_bold", with = "bool_opt_serde")]
    pub show_bold: Option<bool>,
    #[serde(rename = "@show_italic", with = "bool_opt_serde")]
    pub show_italic: Option<bool>,
    #[serde(rename = "@show_underline", with = "bool_opt_serde")]
    pub show_underline: Option<bool>,
    #[serde(rename = "@speed_walk_delay")]
    pub speed_walk_delay: Option<u16>,
    #[serde(rename = "@underline_hyperlinks", with = "bool_opt_serde")]
    pub underline_hyperlinks: Option<bool>,
    #[serde(rename = "@use_custom_link_colour", with = "bool_opt_serde")]
    pub use_custom_link_colour: Option<bool>,
    #[serde(rename = "@use_default_colours", with = "bool_opt_serde")]
    pub use_default_colours: Option<bool>,
    #[serde(rename = "@use_mxp", deserialize_with = "XmlEnum::deserialize_opt")]
    pub use_mxp: Option<UseMxp>,
    #[serde(rename = "@utf_8", with = "bool_opt_serde")]
    pub utf_8: Option<bool>,
    #[serde(rename = "@write_world_name_to_log", with = "bool_opt_serde")]
    pub write_world_name_to_log: Option<bool>,

    // ALPHA OPTIONS
    #[serde(rename = "@auto_log_file_name")]
    pub auto_log_file_name: Option<String>,
    #[serde(rename = "@command_stack_character")]
    pub command_stack_character: Option<String>,
    pub connect_text: Option<String>,
    pub log_file_postamble: Option<String>,
    pub log_file_preamble: Option<String>,
    #[serde(rename = "@log_line_postamble_input")]
    pub log_line_postamble_input: Option<String>,
    #[serde(rename = "@log_line_postamble_notes")]
    pub log_line_postamble_notes: Option<String>,
    #[serde(rename = "@log_line_postamble_output")]
    pub log_line_postamble_output: Option<String>,
    #[serde(rename = "@log_line_preamble_input")]
    pub log_line_preamble_input: Option<String>,
    #[serde(rename = "@log_line_preamble_notes")]
    pub log_line_preamble_notes: Option<String>,
    #[serde(rename = "@log_line_preamble_output")]
    pub log_line_preamble_output: Option<String>,
    #[serde(rename = "@name")]
    pub name: Option<String>,
    #[serde(rename = "@new_activity_sound")]
    pub new_activity_sound: Option<String>,
    #[serde(rename = "@password")]
    pub password: Option<String>,
    #[serde(rename = "@password_base64", with = "bool_opt_serde")]
    pub password_base64: Option<bool>,
    #[serde(rename = "@player")]
    pub player: Option<String>,
    #[serde(rename = "@proxy_server")]
    pub proxy_server: Option<String>,
    #[serde(rename = "@proxy_username")]
    pub proxy_username: Option<String>,
    #[serde(rename = "@proxy_password")]
    pub proxy_password: Option<String>,
    #[serde(rename = "@proxy_password_base64", with = "bool_opt_serde")]
    pub proxy_password_base64: Option<bool>,
    #[serde(rename = "@script_filename")]
    pub script_filename: Option<String>,
    #[serde(rename = "@site")]
    pub site: Option<String>,
    #[serde(rename = "@speed_walk_filler")]
    pub speed_walk_filler: Option<String>,
    #[serde(rename = "@speed_walk_prefix")]
    pub speed_walk_prefix: Option<String>,
    #[serde(rename = "@terminal_identification")]
    pub terminal_identification: Option<String>,
}

impl XmlWorldPartial {
    pub fn apply(self, config: &mut WorldConfig) {
        let Self {
            carriage_return_clears_line,
            connect_method,
            convert_ga_to_newline,
            disable_compression,
            display_my_input,
            echo_colour: _,
            echo_hyperlink_in_output_window,
            enable_aliases,
            enable_command_stack,
            enable_scripts,
            enable_speed_walk,
            enable_timers,
            enable_triggers,
            enable_trigger_sounds,
            hyperlink_adds_to_command_history,
            hyperlink_colour: _,
            ignore_mxp_colour_changes,
            indent_paras,
            keep_commands_on_same_line,
            keypad_enable,
            line_information,
            log_html,
            log_input,
            log_in_colour,
            log_notes,
            log_output,
            log_raw,
            log_script_errors,
            mud_can_change_link_colour,
            mud_can_remove_underline,
            mxp_debug_level,
            naws,
            note_text_colour: _,
            no_echo_off,
            play_sounds_in_background: _,
            port,
            proxy_port,
            proxy_type: _,
            save_world_automatically,
            script_reload_option,
            script_errors_to_output_window,
            show_bold,
            show_italic,
            show_underline,
            speed_walk_delay,
            underline_hyperlinks,
            use_custom_link_colour,
            use_default_colours,
            use_mxp,
            utf_8,
            write_world_name_to_log,
            auto_log_file_name,
            command_stack_character,
            connect_text,
            log_file_postamble,
            log_file_preamble,
            log_line_postamble_input,
            log_line_postamble_notes,
            log_line_postamble_output,
            log_line_preamble_input,
            log_line_preamble_notes,
            log_line_preamble_output,
            name,
            new_activity_sound,
            password,
            password_base64,
            player,
            proxy_server,
            proxy_username,
            proxy_password,
            proxy_password_base64,
            script_filename,
            site,
            speed_walk_filler,
            speed_walk_prefix,
            terminal_identification,
        } = self;
        if let Some(proxy_password) = proxy_password {
            config.proxy_password =
                decode_password(proxy_password, proxy_password_base64.unwrap_or_default());
        }
        if let Some(password) = password {
            config.password = decode_password(password, password_base64.unwrap_or_default());
        }
        match (log_html, log_raw) {
            (None, None) => (),
            (Some(true), _) => config.log_format = LogFormat::Html,
            (_, Some(true)) => config.log_format = LogFormat::Raw,
            _ => config.log_format = LogFormat::Text,
        }
        if let Some(indent_paras) = indent_paras {
            config.indent_paras = indent_paras.into();
        }
        if let Some(speed_walk_prefix) = speed_walk_prefix.as_deref().and_then(first_char) {
            config.speed_walk_prefix = speed_walk_prefix;
        }
        if let Some(speed_walk_delay) = speed_walk_delay {
            config.speed_walk_delay = Duration::from_millis(speed_walk_delay.into());
        }
        if let Some(command_stack_character) =
            command_stack_character.as_deref().and_then(first_char)
        {
            config.command_stack_character = command_stack_character;
        }
        if let Some(script_filename) = script_filename {
            config.world_script = script_filename;
        }
        macro_rules! apply {
            ($i:ident) => {
                if let Some(opt) = $i {
                    config.$i = opt;
                }
            };
        }
        apply!(carriage_return_clears_line);
        apply!(connect_method);
        apply!(convert_ga_to_newline);
        apply!(disable_compression);
        apply!(display_my_input);
        apply!(echo_hyperlink_in_output_window);
        apply!(enable_aliases);
        apply!(enable_command_stack);
        apply!(enable_scripts);
        apply!(enable_speed_walk);
        apply!(enable_timers);
        apply!(enable_triggers);
        apply!(enable_trigger_sounds);
        apply!(hyperlink_adds_to_command_history);
        apply!(ignore_mxp_colour_changes);
        apply!(keep_commands_on_same_line);
        apply!(keypad_enable);
        apply!(line_information);
        apply!(log_input);
        apply!(log_in_colour);
        apply!(log_notes);
        apply!(log_output);
        apply!(log_script_errors);
        apply!(mud_can_change_link_colour);
        apply!(mud_can_remove_underline);
        apply!(mxp_debug_level);
        apply!(naws);
        apply!(no_echo_off);
        apply!(port);
        apply!(proxy_port);
        apply!(save_world_automatically);
        apply!(script_reload_option);
        apply!(script_errors_to_output_window);
        apply!(show_bold);
        apply!(show_italic);
        apply!(show_underline);
        apply!(underline_hyperlinks);
        apply!(use_custom_link_colour);
        apply!(use_default_colours);
        apply!(use_mxp);
        apply!(utf_8);
        apply!(write_world_name_to_log);
        apply!(auto_log_file_name);
        apply!(connect_text);
        apply!(log_file_postamble);
        apply!(log_file_preamble);
        apply!(log_line_postamble_input);
        apply!(log_line_postamble_notes);
        apply!(log_line_postamble_output);
        apply!(log_line_preamble_input);
        apply!(log_line_preamble_notes);
        apply!(log_line_preamble_output);
        apply!(name);
        apply!(new_activity_sound);
        apply!(player);
        apply!(proxy_server);
        apply!(proxy_username);
        apply!(site);
        apply!(speed_walk_filler);
        apply!(terminal_identification);
    }
}
