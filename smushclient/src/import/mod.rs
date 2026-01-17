use std::io::BufRead;

use base64::Engine;
use mud_transformer::UseMxp;
use mud_transformer::mxp::RgbColor;
use serde::Deserialize;
use smushclient_plugins::{Alias, ImportError, RegexError, Timer, Trigger, XmlIterable, XmlVec};

use crate::client::PluginVariables;
use crate::world::{AutoConnect, LogFormat, LogMode, MxpDebugLevel, ScriptRecompile, World};

mod bool_serde {
    pub use serde_this_or_that::as_bool as deserialize;
}

mod color;
use color::Colours;

mod types;
use types::{Include, Key, Variable};

mod xml_enum;
use xml_enum::XmlEnum;

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "world", default)]
struct XmlWorld {
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
    #[serde(rename = "@enable_timers", with = "bool_serde")]
    pub enable_timers: bool,
    #[serde(rename = "@enable_triggers", with = "bool_serde")]
    pub enable_triggers: bool,
    #[serde(rename = "@enable_trigger_sounds", with = "bool_serde")]
    pub enable_trigger_sounds: bool,
    #[serde(rename = "@hyperlink_adds_to_command_history", with = "bool_serde")]
    pub hyperlink_adds_to_command_history: bool,
    #[serde(rename = "@hyperlink_colour")]
    pub hyperlink_colour: RgbColor,
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
    pub note_text_colour: RgbColor,
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
    #[serde(rename = "@terminal_identification")]
    pub terminal_identification: String,
}

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "muclient", default)]
struct MuClient<'a> {
    pub world: XmlWorld,
    #[serde(borrow)]
    pub triggers: XmlVec<<Trigger as XmlIterable>::Xml<'a>>,
    #[serde(borrow)]
    pub aliases: XmlVec<<Alias as XmlIterable>::Xml<'a>>,
    #[serde(borrow)]
    pub timers: XmlVec<<Timer as XmlIterable>::Xml<'a>>,
    // pub macros: XmlVec<Macro>,
    pub variables: XmlVec<Variable>,
    pub colours: Colours,
    pub keypad: XmlVec<Key>,
    // pub printing: Printing,
    pub include: Vec<Include>,
}

#[derive(Debug)]
pub struct ImportedWorld {
    pub world: World,
    pub variables: PluginVariables,
}

impl ImportedWorld {
    pub fn from_xml<R: BufRead>(reader: R) -> Result<Self, ImportError> {
        Ok(
            MuClient::deserialize(&mut quick_xml::de::Deserializer::from_reader(reader))?
                .try_into()?,
        )
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

impl TryFrom<MuClient<'_>> for ImportedWorld {
    type Error = RegexError;

    fn try_from(mut value: MuClient<'_>) -> Result<Self, Self::Error> {
        let custom_colours = value.colours.custom;

        let echo_colour = value
            .world
            .echo_colour
            .and_then(|colour| Some(custom_colours.get(colour)?.text));

        for trigger in &mut value.triggers.elements {
            custom_colours.apply_to_trigger(trigger);
        }

        let world = value.world;

        let world = World {
            name: world.name,
            site: world.site,
            port: world.port,
            use_ssl: false,
            use_proxy: world.proxy_type != 0,
            proxy_server: world.proxy_server,
            proxy_port: world.proxy_port,
            proxy_username: world.proxy_username,
            proxy_password: decode_password(world.proxy_password, world.proxy_password_base64),
            save_world_automatically: world.save_world_automatically,
            player: world.player,
            password: decode_password(world.password, world.password_base64),
            connect_method: world.connect_method,
            connect_text: world.connect_text,
            log_file_preamble: world.log_file_preamble,
            log_file_postamble: world.log_file_postamble,
            log_format: if world.log_html {
                LogFormat::Html
            } else if world.log_raw {
                LogFormat::Raw
            } else {
                LogFormat::Text
            },
            log_in_colour: world.log_in_colour,
            log_output: world.log_output,
            log_input: world.log_input,
            log_notes: world.log_notes,
            log_mode: LogMode::Append,
            auto_log_file_name: world.auto_log_file_name,
            write_world_name_to_log: world.write_world_name_to_log,
            log_line_preamble_output: world.log_line_preamble_output,
            log_line_preamble_input: world.log_line_preamble_input,
            log_line_preamble_notes: world.log_line_preamble_notes,
            log_line_postamble_output: world.log_line_postamble_output,
            log_line_postamble_input: world.log_line_postamble_input,
            log_line_postamble_notes: world.log_line_postamble_notes,
            log_script_errors: world.log_script_errors,
            timers: value.timers.try_into().unwrap(),
            enable_timers: world.enable_timers,
            show_bold: world.show_bold,
            show_italic: world.show_italic,
            show_underline: world.show_underline,
            indent_paras: world.indent_paras.into(),
            ansi_colours: value.colours.ansi,
            use_default_colours: world.use_default_colours,
            display_my_input: world.display_my_input,
            echo_colour,
            echo_background_colour: None,
            keep_commands_on_same_line: world.keep_commands_on_same_line,
            new_activity_sound: world.new_activity_sound,
            line_information: world.line_information,
            use_mxp: world.use_mxp,
            ignore_mxp_colour_changes: world.ignore_mxp_colour_changes,
            use_custom_link_colour: world.use_custom_link_colour,
            hyperlink_colour: world.hyperlink_colour,
            mud_can_change_link_colour: world.mud_can_change_link_colour,
            underline_hyperlinks: world.underline_hyperlinks,
            mud_can_remove_underline: world.mud_can_remove_underline,
            hyperlink_adds_to_command_history: world.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: world.echo_hyperlink_in_output_window,
            terminal_identification: world.terminal_identification,
            disable_compression: world.disable_compression,
            naws: world.naws,
            carriage_return_clears_line: world.carriage_return_clears_line,
            utf_8: world.utf_8,
            convert_ga_to_newline: world.convert_ga_to_newline,
            no_echo_off: world.no_echo_off,
            enable_command_stack: world.enable_command_stack,
            command_stack_character: world
                .command_stack_character
                .as_bytes()
                .first()
                .copied()
                .unwrap_or(b';'),
            mxp_debug_level: world.mxp_debug_level,
            triggers: value.triggers.try_into()?,
            enable_triggers: world.enable_triggers,
            enable_trigger_sounds: world.enable_trigger_sounds,
            aliases: value.aliases.try_into()?,
            enable_aliases: world.enable_aliases,
            numpad_shortcuts: value.keypad.into(),
            keypad_enable: world.keypad_enable,
            hotkey_adds_to_command_history: false,
            echo_hotkey_in_output_window: true,
            enable_scripts: world.enable_scripts,
            world_script: world.script_filename,
            script_reload_option: world.script_reload_option,
            note_text_colour: Some(world.note_text_colour),
            note_background_colour: None,
            script_errors_to_output_window: world.script_errors_to_output_window,
            error_text_colour: Some(RgbColor::rgb(127, 0, 0)),
            error_background_colour: None,
            plugins: value
                .include
                .into_iter()
                .filter(|include| include.plugin)
                .map(|include| include.name.into())
                .collect(),
        };
        Ok(Self {
            world,
            variables: value.variables.into(),
        })
    }
}
