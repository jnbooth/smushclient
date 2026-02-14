use std::slice;

use mud_transformer::UseMxp;
use mud_transformer::mxp::RgbColor;

use super::World;
use super::error::SetOptionError;
use super::types::{AutoConnect, LogFormat, MxpDebugLevel, ScriptRecompile};
use crate::{LuaStr, LuaString};

const fn code_color(code: i64) -> Result<RgbColor, SetOptionError> {
    if code < 0 || code > 0xFFFFFF {
        return Err(SetOptionError::OptionOutOfRange);
    }
    #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
    Ok(RgbColor::hex(code as u32))
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum OptionCaller {
    #[default]
    WorldScript,
    Plugin,
}

impl World {
    pub const INT_OPTIONS: &[&str] = &[
        "carriage_return_clears_line",
        "convert_ga_to_newline",
        "connect_method",
        "disable_compression",
        "display_my_input",
        "echo_hyperlink_in_output_window",
        "enable_aliases",
        "enable_command_stack",
        "enable_scripts",
        "enable_timers",
        "enable_triggers",
        "enable_trigger_sounds",
        "hyperlink_adds_to_command_history",
        "hyperlink_colour",
        "ignore_mxp_colour_changes",
        "indent_paras",
        "keep_commands_on_same_line",
        "keypad_enable",
        "line_information",
        "log_html",
        "log_input",
        "log_in_colour",
        "log_notes",
        "log_output",
        "log_raw",
        "log_script_errors",
        "mud_can_change_link_colour",
        "mud_can_remove_underline",
        "mxp_debug_level",
        "naws",
        "note_text_colour",
        "no_echo_off",
        "port",
        "proxy_port",
        "save_world_automatically",
        "script_errors_to_output_window",
        "script_reload_option",
        "show_bold",
        "show_italic",
        "show_underline",
        "underline_hyperlinks",
        "use_custom_link_colour",
        "use_default_colours",
        "use_mxp",
        "utf_8",
        "write_world_name_to_log",
    ];

    pub fn option_int(&self, _: OptionCaller, option: &LuaStr) -> Option<i64> {
        Some(match option {
            b"carriage_return_clears_line" => self.carriage_return_clears_line.into(),
            b"convert_ga_to_newline" => self.convert_ga_to_newline.into(),
            b"connect_method" => self.connect_method as _,
            b"disable_compression" => self.disable_compression.into(),
            b"display_my_input" => self.display_my_input.into(),
            b"echo_hyperlink_in_output_window" => self.echo_hyperlink_in_output_window.into(),
            b"enable_aliases" => self.enable_aliases.into(),
            b"enable_command_stack" => self.enable_command_stack.into(),
            b"enable_scripts" => self.enable_scripts.into(),
            b"enable_timers" => self.enable_timers.into(),
            b"enable_triggers" => self.enable_triggers.into(),
            b"enable_trigger_sounds" => self.enable_trigger_sounds.into(),
            b"hyperlink_adds_to_command_history" => self.hyperlink_adds_to_command_history.into(),
            b"hyperlink_colour" => self.hyperlink_colour.code().into(),
            b"ignore_mxp_colour_changes" => self.ignore_mxp_colour_changes.into(),
            b"indent_paras" => self.indent_paras.into(),
            b"keep_commands_on_same_line" => self.keep_commands_on_same_line.into(),
            b"keypad_enable" => self.keypad_enable.into(),
            b"line_information" => self.line_information.into(),
            b"log_html" => (self.log_format == LogFormat::Html).into(),
            b"log_input" => self.log_input.into(),
            b"log_in_colour" => self.log_in_colour.into(),
            b"log_notes" => self.log_notes.into(),
            b"log_output" => self.log_output.into(),
            b"log_raw" => (self.log_format == LogFormat::Raw).into(),
            b"log_script_errors" => self.log_script_errors.into(),
            b"mud_can_change_link_colour" => self.mud_can_change_link_colour.into(),
            b"mud_can_remove_underline" => self.mud_can_remove_underline.into(),
            b"mxp_debug_level" => self.mxp_debug_level as _,
            b"naws" => self.naws.into(),
            b"note_text_colour" => self.note_text_colour.unwrap_or_default().code().into(),
            b"no_echo_off" => self.no_echo_off.into(),
            b"port" => self.port.into(),
            b"proxy_port" => self.proxy_port.into(),
            b"save_world_automatically" => self.save_world_automatically.into(),
            b"script_errors_to_output_window" => self.script_errors_to_output_window.into(),
            b"script_reload_option" => self.script_reload_option as _,
            b"show_bold" => self.show_bold.into(),
            b"show_italic" => self.show_italic.into(),
            b"show_underline" => self.show_underline.into(),
            b"underline_hyperlinks" => self.underline_hyperlinks.into(),
            b"use_custom_link_colour" => self.use_custom_link_colour.into(),
            b"use_default_colours" => self.use_default_colours.into(),
            b"use_mxp" => self.use_mxp as _,
            b"utf_8" => self.utf_8.into(),
            b"write_world_name_to_log" => self.write_world_name_to_log.into(),
            _ => return None,
        })
    }

    pub fn set_option_int(
        &mut self,
        caller: OptionCaller,
        option: &LuaStr,
        value: i64,
    ) -> Result<(), SetOptionError> {
        let on = match value {
            0 => Ok(false),
            1 => Ok(true),
            _ => Err(SetOptionError::OptionOutOfRange),
        };

        match option {
            b"carriage_return_clears_line" => self.carriage_return_clears_line = on?,
            b"convert_ga_to_newline" => self.convert_ga_to_newline = on?,
            b"connect_method" => {
                self.connect_method = match value {
                    0 => AutoConnect::None,
                    1 => AutoConnect::Mush,
                    2 => AutoConnect::Diku,
                    3 => AutoConnect::Mxp,
                    _ => return Err(SetOptionError::OptionOutOfRange),
                }
            }
            b"disable_compression" => self.disable_compression = on?,
            b"display_my_input" => self.display_my_input = on?,
            b"echo_hyperlink_in_output_window" => self.echo_hyperlink_in_output_window = on?,
            b"enable_aliases" => self.enable_aliases = on?,
            b"enable_command_stack" => self.enable_command_stack = on?,
            b"enable_scripts" => self.enable_scripts = on?,
            b"enable_timers" => self.enable_timers = on?,
            b"enable_triggers" => self.enable_triggers = on?,
            b"enable_trigger_sounds" => self.enable_trigger_sounds = on?,
            b"hyperlink_adds_to_command_history" => self.hyperlink_adds_to_command_history = on?,
            b"hyperlink_colour" => self.hyperlink_colour = code_color(value)?,
            b"ignore_mxp_colour_changes" => self.ignore_mxp_colour_changes = on?,
            b"indent_paras" => self.indent_paras = u8::try_from(value)?,
            b"keep_commands_on_same_line" => self.keep_commands_on_same_line = on?,
            b"keypad_enable" => self.keypad_enable = on?,
            b"line_information" => self.line_information = on?,
            b"log_html" => {
                if on? {
                    self.log_format = LogFormat::Html;
                } else if self.log_format == LogFormat::Html {
                    self.log_format = LogFormat::Text;
                }
            }
            b"log_input" => self.log_input = on?,
            b"log_in_colour" => self.log_in_colour = on?,
            b"log_notes" => self.log_notes = on?,
            b"log_output" => self.log_output = on?,
            b"log_raw" => {
                if on? {
                    self.log_format = LogFormat::Raw;
                } else if self.log_format == LogFormat::Raw {
                    self.log_format = LogFormat::Text;
                }
            }
            b"log_script_errors" => self.log_script_errors = on?,
            b"mud_can_change_link_colour" => self.mud_can_change_link_colour = on?,
            b"mud_can_remove_underline" => self.mud_can_remove_underline = on?,
            b"mxp_debug_level" => {
                self.mxp_debug_level = match value {
                    0 => MxpDebugLevel::None,
                    1 => MxpDebugLevel::Error,
                    2 => MxpDebugLevel::Warning,
                    3 => MxpDebugLevel::Info,
                    4 => MxpDebugLevel::All,
                    _ => return Err(SetOptionError::OptionOutOfRange),
                }
            }
            b"naws" => self.naws = on?,
            b"note_text_colour" => self.note_text_colour = Some(code_color(value)?),
            b"no_echo_off" => self.no_echo_off = on?,
            b"save_world_automatically" => self.save_world_automatically = on?,
            b"script_errors_to_output_window" => self.script_errors_to_output_window = on?,
            b"script_reload_option" => {
                self.script_reload_option = match value {
                    0 => ScriptRecompile::Confirm,
                    1 => ScriptRecompile::Always,
                    2 => ScriptRecompile::Never,
                    _ => return Err(SetOptionError::OptionOutOfRange),
                }
            }
            b"show_bold" => self.show_bold = on?,
            b"show_italic" => self.show_italic = on?,
            b"show_underline" => self.show_underline = on?,
            b"underline_hyperlinks" => self.underline_hyperlinks = on?,
            b"use_custom_link_colour" => self.use_custom_link_colour = on?,
            b"use_default_colours" => self.use_default_colours = on?,
            b"use_mxp" => {
                self.use_mxp = match value {
                    0 => UseMxp::Command,
                    1 => UseMxp::Query,
                    2 => UseMxp::Always,
                    3 => UseMxp::Never,
                    _ => return Err(SetOptionError::OptionOutOfRange),
                }
            }
            b"utf_8" => self.utf_8 = on?,
            b"write_world_name_to_log" => self.write_world_name_to_log = on?,
            _ if caller == OptionCaller::Plugin => {
                return Err(SetOptionError::PluginCannotSetOption);
            }
            b"port" => self.port = u16::try_from(value)?,
            b"proxy_port" => self.proxy_port = u16::try_from(value)?,
            _ => return Err(SetOptionError::UnknownOption),
        }
        Ok(())
    }

    pub const STR_OPTIONS: &[&str] = &[
        "auto_log_file_name",
        "command_stack_character",
        "connect_text",
        "log_file_postamble",
        "log_file_preamble",
        "log_line_postamble_input",
        "log_line_postamble_notes",
        "log_line_postamble_output",
        "log_line_preamble_input",
        "log_line_preamble_notes",
        "log_line_preamble_output",
        "name",
        "new_activity_sound",
        "password",
        "player",
        "proxy_server",
        "proxy_username",
        "proxy_password",
        "site",
        "terminal_identification",
    ];

    pub fn option_str(&self, caller: OptionCaller, option: &LuaStr) -> Option<&LuaStr> {
        Some(
            match option {
                b"auto_log_file_name" => &self.auto_log_file_name,
                b"command_stack_character" => {
                    return Some(slice::from_ref(&self.command_stack_character));
                }
                b"connect_text" => &self.connect_text,
                b"log_file_postamble" => &self.log_file_postamble,
                b"log_file_preamble" => &self.log_file_preamble,
                b"log_line_postamble_input" => &self.log_line_postamble_input,
                b"log_line_postamble_notes" => &self.log_line_postamble_notes,
                b"log_line_postamble_output" => &self.log_line_postamble_output,
                b"log_line_preamble_input" => &self.log_line_preamble_input,
                b"log_line_preamble_notes" => &self.log_line_preamble_notes,
                b"log_line_preamble_output" => &self.log_line_preamble_output,
                b"name" => &self.name,
                b"new_activity_sound" => &self.new_activity_sound,
                b"player" => &self.player,
                b"proxy_username" => &self.proxy_username,
                b"site" => &self.site,
                b"terminal_identification" => &self.terminal_identification,
                _ if caller == OptionCaller::Plugin => return None,
                b"password" => &self.password,
                b"proxy_password" => &self.proxy_password,
                b"proxy_server" => &self.proxy_server,
                _ => return None,
            }
            .as_bytes(),
        )
    }

    pub fn set_option_str(
        &mut self,
        caller: OptionCaller,
        option: &LuaStr,
        value: LuaString,
    ) -> Result<(), SetOptionError> {
        let value = String::from_utf8(value)?;
        match option {
            b"auto_log_file_name" => self.auto_log_file_name = value,
            b"command_stack_character" => match value.as_bytes() {
                [c] if c.is_ascii() => self.command_stack_character = *c,
                _ => return Err(SetOptionError::OptionOutOfRange),
            },
            b"connect_text" => self.connect_text = value,
            b"log_file_postamble" => self.log_file_postamble = value,
            b"log_file_preamble" => self.log_file_preamble = value,
            b"log_line_postamble_input" => self.log_line_postamble_input = value,
            b"log_line_postamble_notes" => self.log_line_postamble_notes = value,
            b"log_line_postamble_output" => self.log_line_postamble_output = value,
            b"log_line_preamble_input" => self.log_line_preamble_input = value,
            b"log_line_preamble_notes" => self.log_line_preamble_notes = value,
            b"log_line_preamble_output" => self.log_line_preamble_output = value,
            b"new_activity_sound" => self.new_activity_sound = value,
            b"terminal_identification" => self.terminal_identification = value,
            _ if caller == OptionCaller::Plugin => {
                return Err(SetOptionError::PluginCannotSetOption);
            }
            b"name" => self.name = value,
            b"password" => self.password = value,
            b"player" => self.player = value,
            b"proxy_server" => self.proxy_server = value,
            b"proxy_username" => self.proxy_username = value,
            b"proxy_password" => self.proxy_password = value,
            b"site" => self.site = value,
            _ => return Err(SetOptionError::UnknownOption),
        }
        Ok(())
    }
}
