#![allow(clippy::result_large_err)]
use std::collections::HashMap;
use std::error::Error;
use std::pin::Pin;

use crate::adapter::TreeBuilderAdapter;
use crate::colors::Colors;
use crate::convert::Convert;
use crate::ffi;
use crate::sender::{AliasRust, TimerRust, TriggerRust};
use cxx_qt_lib::{QColor, QString};
use smushclient::world::ColorPair;
use smushclient::World;
use smushclient_plugins::{Alias, Occurrence, PluginLoadError, RegexError, Sender, Timer, Trigger};

#[derive(Default)]
pub struct WorldRust {
    // IP address
    pub name: QString,
    pub site: QString,
    pub port: i32,
    pub proxy_type: ffi::ProxyType,
    pub proxy_server: QString,
    pub proxy_port: i32,
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
    pub chat_port: i32,
    pub validate_incoming_chat_calls: bool,
    pub chat_foreground_colour: QColor,
    pub chat_background_colour: QColor,
    pub ignore_chat_colours: bool,
    pub chat_message_prefix: QString,
    pub chat_max_lines_per_message: i32,
    pub chat_max_bytes_per_message: i32,
    pub auto_allow_files: bool,
    pub chat_file_save_directory: QString,

    // Notes
    pub notes: QString,

    // Output
    pub beep_sound: QString,
    pub pixel_offset: i32,
    pub line_spacing: f64,
    pub output_font: QString,
    pub output_font_height: i32,
    pub use_default_output_font: bool,
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub new_activity_sound: QString,
    pub max_output_lines: i32,
    pub wrap_column: i32,

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

    // MXP
    pub use_mxp: ffi::UseMxp,
    pub hyperlink_colour: QColor,
    pub use_custom_link_colour: bool,
    pub mud_can_change_link_colour: bool,
    pub underline_hyperlinks: bool,
    pub mud_can_remove_underline: bool,
    pub hyperlink_adds_to_command_history: bool,
    pub echo_hyperlink_in_output_window: bool,
    pub ignore_mxp_colour_changes: bool,
    pub send_mxp_afk_response: bool,

    // ANSI Color
    pub ansi_colors: Colors,
    pub use_default_colours: bool,
    pub custom_color: QColor,
    pub error_colour: QColor,

    // Triggers
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,
    pub enable_trigger_sounds: bool,
    pub treeview_triggers: bool,

    // Commands
    pub display_my_input: bool,
    pub echo_text_colour: QColor,
    pub echo_background_colour: QColor,
    pub enable_speed_walk: bool,
    pub speed_walk_prefix: QString,
    pub speed_walk_filler: QString,
    pub speed_walk_delay: f64,
    pub enable_command_stack: bool,
    pub command_stack_character: QString,
    pub input_text_colour: QColor,
    pub input_background_colour: QColor,
    pub input_font: QString,
    pub input_font_height: i32,
    pub use_default_input_font: bool,
    pub enable_spam_prevention: bool,
    pub spam_line_count: i32,
    pub spam_message: QString,

    pub auto_repeat: bool,
    pub lower_case_tab_completion: bool,
    pub translate_german: bool,
    pub translate_backslash_sequences: bool,
    pub keep_commands_on_same_line: bool,
    pub no_echo_off: bool,
    pub tab_completion_lines: i32,
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
    pub history_lines: i32,

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
    pub paste_delay: i32,
    pub paste_delay_per_lines: i32,
    pub paste_commented_softcode: bool,
    pub paste_echo: bool,
    pub confirm_on_paste: bool,

    // Send
    pub send_line_preamble: QString,
    pub send_line_postamble: QString,
    pub send_file_delay: i32,
    pub send_file_delay_per_lines: i32,
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
    pub note_text_colour: QColor,
}

impl_deref!(WorldRust, Colors, ansi_colors);

fn sort_refs<T: AsRef<Sender>>(items: &[T]) -> Vec<(usize, &T)> {
    let mut sorted_items: Vec<(usize, &T)> = items.iter().enumerate().collect();
    sorted_items.sort_unstable_by_key(|(_, item)| (*item).as_ref());
    sorted_items
}

impl WorldRust {
    pub fn add_alias(&mut self, alias: &AliasRust) -> Result<(), RegexError> {
        self.aliases.push(Alias::try_from(alias)?);
        Ok(())
    }

    pub fn add_timer(&mut self, timer: &TimerRust) {
        self.timers.push(Timer::from(timer));
    }

    pub fn add_trigger(&mut self, trigger: &TriggerRust) -> Result<(), RegexError> {
        self.triggers.push(Trigger::try_from(trigger)?);
        Ok(())
    }

    pub fn export_aliases(&self) -> Result<String, PluginLoadError> {
        Alias::to_xml_string(&self.aliases)
    }

    pub fn export_timers(&self) -> Result<String, PluginLoadError> {
        Timer::to_xml_string(&self.timers)
    }

    pub fn export_triggers(&self) -> Result<String, PluginLoadError> {
        Trigger::to_xml_string(&self.triggers)
    }

    pub fn import_aliases(&mut self, xml: &str) -> Result<(), PluginLoadError> {
        self.aliases.append(&mut Alias::from_xml_str(xml)?);
        Ok(())
    }

    pub fn import_timers(&mut self, xml: &str) -> Result<(), PluginLoadError> {
        self.timers.append(&mut Timer::from_xml_str(xml)?);
        Ok(())
    }

    pub fn import_triggers(&mut self, xml: &str) -> Result<(), PluginLoadError> {
        self.triggers.append(&mut Trigger::from_xml_str(xml)?);
        Ok(())
    }

    pub fn remove_alias(&mut self, index: usize) {
        if index < self.aliases.len() {
            self.aliases.remove(index);
        }
    }

    pub fn remove_timer(&mut self, index: usize) {
        if index < self.timers.len() {
            self.timers.remove(index);
        }
    }

    pub fn remove_trigger(&mut self, index: usize) {
        if index < self.triggers.len() {
            self.triggers.remove(index);
        }
    }

    pub fn replace_alias(&mut self, index: usize, alias: &AliasRust) -> Result<(), RegexError> {
        let Some(entry) = self.aliases.get_mut(index) else {
            return self.add_alias(alias);
        };
        *entry = Alias::try_from(alias)?;
        Ok(())
    }

    pub fn replace_timer(&mut self, index: usize, timer: &TimerRust) {
        let Some(entry) = self.timers.get_mut(index) else {
            self.add_timer(timer);
            return;
        };
        *entry = Timer::from(timer);
    }

    pub fn replace_trigger(
        &mut self,
        index: usize,
        trigger: &TriggerRust,
    ) -> Result<(), RegexError> {
        let Some(entry) = self.triggers.get_mut(index) else {
            return self.add_trigger(trigger);
        };
        *entry = Trigger::try_from(trigger)?;
        Ok(())
    }

    pub fn build_alias_tree(&self, builder: &mut TreeBuilderAdapter) {
        if self.aliases.is_empty() {
            return;
        }
        let sorted_items = sort_refs(&self.aliases);
        let mut last_group = "";
        for (index, item) in sorted_items {
            let group = item.group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.start_item(index);
            builder.add_column(&QString::from(&item.label));
            builder.add_column(i64::from(item.sequence));
            builder.add_column(&QString::from(&item.pattern));
            builder.add_column(&QString::from(&item.text));
        }
    }

    pub fn build_timer_tree(&self, builder: &mut TreeBuilderAdapter) {
        if self.timers.is_empty() {
            return;
        }
        let sorted_items = sort_refs(&self.timers);
        let mut last_group = "";
        let at = QString::from("At");
        let every = QString::from("Every");
        for (index, item) in sorted_items {
            let group = item.group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.start_item(index);
            builder.add_column(&QString::from(&item.label));
            builder.add_column(match item.occurrence {
                Occurrence::Interval(_) => &every,
                Occurrence::Time(_) => &at,
            });
            builder.add_column(&QString::from(&item.occurrence.to_string()));
            builder.add_column(&QString::from(&item.text));
        }
    }

    pub fn build_trigger_tree(&self, builder: &mut TreeBuilderAdapter) {
        if self.triggers.is_empty() {
            return;
        }
        let sorted_items = sort_refs(&self.triggers);
        let mut last_group = "";
        for (index, item) in sorted_items {
            let group = item.group.as_str();
            if group != last_group {
                builder.start_group(&QString::from(group));
                last_group = group;
            }
            builder.start_item(index);
            builder.add_column(&QString::from(&item.label));
            builder.add_column(i64::from(item.sequence));
            builder.add_column(&QString::from(&item.pattern));
            builder.add_column(&QString::from(&item.text));
        }
    }
}

#[inline(always)]
fn sign_u32(val: u32) -> i32 {
    i32::try_from(val).unwrap_or(i32::MAX)
}

#[inline(always)]
fn sign_usize(val: usize) -> i32 {
    i32::try_from(val).unwrap_or(i32::MAX)
}

impl From<&World> for WorldRust {
    fn from(world: &World) -> Self {
        Self {
            name: QString::from(&world.name),
            site: QString::from(&world.site),
            port: i32::from(world.port),
            proxy_type: world.proxy_type.into(),
            proxy_server: QString::from(&world.proxy_server),
            proxy_port: i32::from(world.proxy_port),
            proxy_username: QString::from(&world.proxy_username),
            proxy_password: QString::from(&world.proxy_password),
            proxy_password_base64: world.proxy_password_base64,
            save_world_automatically: world.save_world_automatically,
            player: QString::from(&world.player),
            password: QString::from(&world.password),
            connect_method: world.connect_method.into(),
            connect_text: QString::from(&world.connect_text),
            log_file_preamble: QString::from(&world.log_file_preamble),
            log_file_postamble: QString::from(&world.log_file_postamble),
            log_format: world.log_format.into(),
            log_output: world.log_output,
            log_input: world.log_input,
            log_notes: world.log_notes,
            log_mode: world.log_mode.into(),
            auto_log_file_name: world.auto_log_file_name.convert(),
            log_preamble_output: QString::from(&world.log_preamble_output),
            log_preamble_input: QString::from(&world.log_preamble_input),
            log_preamble_notes: QString::from(&world.log_preamble_notes),
            log_postamble_output: QString::from(&world.log_postamble_output),
            log_postamble_input: QString::from(&world.log_postamble_input),
            log_postamble_notes: QString::from(&world.log_postamble_notes),
            timers: world.timers.clone(),
            enable_timers: world.enable_timers,
            treeview_timers: world.treeview_timers,
            chat_name: QString::from(&world.chat_name),
            auto_allow_snooping: world.auto_allow_snooping,
            accept_chat_connections: world.accept_chat_connections,
            chat_port: i32::from(world.chat_port),
            validate_incoming_chat_calls: world.validate_incoming_chat_calls,
            chat_foreground_colour: world.chat_colors.foreground.convert(),
            chat_background_colour: world.chat_colors.background.convert(),
            ignore_chat_colours: world.ignore_chat_colours,
            chat_message_prefix: QString::from(&world.chat_message_prefix),
            chat_max_lines_per_message: sign_usize(world.chat_max_lines_per_message),
            chat_max_bytes_per_message: sign_usize(world.chat_max_bytes_per_message),
            auto_allow_files: world.auto_allow_files,
            chat_file_save_directory: world.chat_file_save_directory.convert(),
            notes: QString::from(&world.notes),
            beep_sound: world.beep_sound.convert(),
            pixel_offset: i32::from(world.pixel_offset),
            line_spacing: world.line_spacing,
            output_font: QString::from(&world.output_font),
            output_font_height: i32::from(world.output_font_height),
            use_default_output_font: world.use_default_output_font,
            show_bold: world.show_bold,
            show_italic: world.show_italic,
            show_underline: world.show_underline,
            new_activity_sound: world.new_activity_sound.convert(),
            max_output_lines: sign_usize(world.max_output_lines),
            wrap_column: i32::from(world.wrap_column),
            line_information: world.line_information,
            start_paused: world.start_paused,
            auto_pause: world.auto_pause,
            unpause_on_send: world.unpause_on_send,
            flash_taskbar_icon: world.flash_taskbar_icon,
            disable_compression: world.disable_compression,
            indent_paras: world.indent_paras,
            naws: world.naws,
            carriage_return_clears_line: world.carriage_return_clears_line,
            utf_8: world.utf_8,
            auto_wrap_window_width: world.auto_wrap_window_width,
            show_connect_disconnect: world.show_connect_disconnect,
            copy_selection_to_clipboard: world.copy_selection_to_clipboard,
            auto_copy_to_clipboard_in_html: world.auto_copy_to_clipboard_in_html,
            convert_ga_to_newline: world.convert_ga_to_newline,
            terminal_identification: QString::from(&world.terminal_identification),
            use_mxp: world.use_mxp.into(),
            hyperlink_colour: world.hyperlink_colour.convert(),
            use_custom_link_colour: world.use_custom_link_colour,
            mud_can_change_link_colour: world.mud_can_change_link_colour,
            underline_hyperlinks: world.underline_hyperlinks,
            mud_can_remove_underline: world.mud_can_remove_underline,
            hyperlink_adds_to_command_history: world.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: world.echo_hyperlink_in_output_window,
            ignore_mxp_colour_changes: world.ignore_mxp_colour_changes,
            send_mxp_afk_response: world.send_mxp_afk_response,
            ansi_colors: Colors::from(&world.ansi_colors),
            use_default_colours: world.use_default_colours,
            custom_color: world.custom_color.convert(),
            error_colour: world.error_colour.convert(),
            triggers: world.triggers.clone(),
            enable_triggers: world.enable_triggers,
            enable_trigger_sounds: world.enable_trigger_sounds,
            treeview_triggers: world.treeview_triggers,
            display_my_input: world.display_my_input,
            echo_text_colour: world.echo_colors.foreground.convert(),
            echo_background_colour: world.echo_colors.background.convert(),
            enable_speed_walk: world.enable_speed_walk,
            speed_walk_prefix: QString::from(&world.speed_walk_prefix),
            speed_walk_filler: QString::from(&world.speed_walk_filler),
            speed_walk_delay: world.speed_walk_delay,
            enable_command_stack: world.enable_command_stack,
            command_stack_character: QString::from(&world.command_stack_character),
            input_text_colour: world.input_colors.foreground.convert(),
            input_background_colour: world.input_colors.background.convert(),
            input_font: QString::from(&world.input_font),
            input_font_height: i32::from(world.input_font_height),
            use_default_input_font: world.use_default_input_font,
            enable_spam_prevention: world.enable_spam_prevention,
            spam_line_count: sign_usize(world.spam_line_count),
            spam_message: QString::from(&world.spam_message),
            auto_repeat: world.auto_repeat,
            lower_case_tab_completion: world.lower_case_tab_completion,
            translate_german: world.translate_german,
            translate_backslash_sequences: world.translate_backslash_sequences,
            keep_commands_on_same_line: world.keep_commands_on_same_line,
            no_echo_off: world.no_echo_off,
            tab_completion_lines: sign_usize(world.tab_completion_lines),
            tab_completion_space: world.tab_completion_space,
            double_click_inserts: world.double_click_inserts,
            double_click_sends: world.double_click_sends,
            escape_deletes_input: world.escape_deletes_input,
            save_deleted_command: world.save_deleted_command,
            confirm_before_replacing_typing: world.confirm_before_replacing_typing,
            arrow_keys_wrap: world.arrow_keys_wrap,
            arrows_change_history: world.arrows_change_history,
            arrow_recalls_partial: world.arrow_recalls_partial,
            alt_arrow_recalls_partial: world.alt_arrow_recalls_partial,
            ctrl_z_goes_to_end_of_buffer: world.ctrl_z_goes_to_end_of_buffer,
            ctrl_p_goes_to_previous_command: world.ctrl_p_goes_to_previous_command,
            ctrl_n_goes_to_next_command: world.ctrl_n_goes_to_next_command,
            history_lines: sign_usize(world.history_lines),
            aliases: world.aliases.clone(),
            enable_aliases: world.enable_aliases,
            treeview_aliases: world.treeview_aliases,
            keypad_enable: world.keypad_enable,
            keypad_shortcuts: world.keypad_shortcuts.clone(),
            enable_auto_say: world.enable_auto_say,
            autosay_exclude_non_alpha: world.autosay_exclude_non_alpha,
            autosay_exclude_macros: world.autosay_exclude_macros,
            auto_say_override_prefix: QString::from(&world.auto_say_override_prefix),
            auto_say_string: QString::from(&world.auto_say_string),
            re_evaluate_auto_say: world.re_evaluate_auto_say,
            paste_line_preamble: QString::from(&world.paste_line_preamble),
            paste_line_postamble: QString::from(&world.paste_line_postamble),
            paste_delay: sign_u32(world.paste_delay),
            paste_delay_per_lines: sign_u32(world.paste_delay_per_lines),
            paste_commented_softcode: world.paste_commented_softcode,
            paste_echo: world.paste_echo,
            confirm_on_paste: world.confirm_on_paste,
            send_line_preamble: QString::from(&world.send_line_preamble),
            send_line_postamble: QString::from(&world.send_line_postamble),
            send_file_delay: sign_u32(world.send_file_delay),
            send_file_delay_per_lines: sign_u32(world.send_file_delay_per_lines),
            send_commented_softcode: world.send_commented_softcode,
            send_echo: world.send_echo,
            confirm_on_send: world.confirm_on_send,
            world_script: QString::from(&world.world_script),
            script_prefix: QString::from(&world.script_prefix),
            enable_scripts: world.enable_scripts,
            warn_if_scripting_inactive: world.warn_if_scripting_inactive,
            edit_script_with_notepad: world.edit_script_with_notepad,
            script_editor: QString::from(&world.script_editor),
            script_reload_option: world.script_reload_option.into(),
            script_errors_to_output_window: world.script_errors_to_output_window,
            note_text_colour: world.note_text_colour.convert(),
        }
    }
}

impl TryFrom<&WorldRust> for World {
    type Error = crate::convert::OutOfRangeError;

    fn try_from(value: &WorldRust) -> Result<Self, Self::Error> {
        let mut world = Self {
            name: String::from(&value.name),
            site: String::from(&value.site),
            port: u16::try_from(value.port)?,
            proxy_type: value.proxy_type.try_into()?,
            proxy_server: String::from(&value.proxy_server),
            proxy_port: u16::try_from(value.proxy_port)?,
            proxy_username: String::from(&value.proxy_username),
            proxy_password: String::from(&value.proxy_password),
            proxy_password_base64: value.proxy_password_base64,
            save_world_automatically: value.save_world_automatically,
            player: String::from(&value.player),
            password: String::from(&value.password),
            connect_method: value.connect_method.try_into()?,
            connect_text: String::from(&value.connect_text),
            log_file_preamble: String::from(&value.log_file_preamble),
            log_file_postamble: String::from(&value.log_file_postamble),
            log_format: value.log_format.try_into()?,
            log_output: value.log_output,
            log_input: value.log_input,
            log_notes: value.log_notes,
            log_mode: value.log_mode.try_into()?,
            auto_log_file_name: value.auto_log_file_name.convert(),
            log_preamble_output: String::from(&value.log_preamble_output),
            log_preamble_input: String::from(&value.log_preamble_input),
            log_preamble_notes: String::from(&value.log_preamble_notes),
            log_postamble_output: String::from(&value.log_postamble_output),
            log_postamble_input: String::from(&value.log_postamble_input),
            log_postamble_notes: String::from(&value.log_postamble_notes),
            timers: value.timers.clone(),
            enable_timers: value.enable_timers,
            treeview_timers: value.treeview_timers,
            chat_name: String::from(&value.chat_name),
            auto_allow_snooping: value.auto_allow_snooping,
            accept_chat_connections: value.accept_chat_connections,
            chat_port: u16::try_from(value.chat_port)?,
            validate_incoming_chat_calls: value.validate_incoming_chat_calls,
            chat_colors: ColorPair {
                foreground: value.chat_foreground_colour.convert(),
                background: value.chat_background_colour.convert(),
            },
            ignore_chat_colours: value.ignore_chat_colours,
            chat_message_prefix: String::from(&value.chat_message_prefix),
            chat_max_lines_per_message: usize::try_from(value.chat_max_lines_per_message)?,
            chat_max_bytes_per_message: usize::try_from(value.chat_max_bytes_per_message)?,
            auto_allow_files: value.auto_allow_files,
            chat_file_save_directory: value.chat_file_save_directory.convert(),
            notes: String::from(&value.notes),
            beep_sound: value.beep_sound.convert(),
            pixel_offset: i16::try_from(value.pixel_offset)?,
            line_spacing: value.line_spacing,
            output_font: String::from(&value.output_font),
            output_font_height: u8::try_from(value.output_font_height)?,
            use_default_output_font: value.use_default_output_font,
            show_bold: value.show_bold,
            show_italic: value.show_italic,
            show_underline: value.show_underline,
            new_activity_sound: value.new_activity_sound.convert(),
            max_output_lines: usize::try_from(value.max_output_lines)?,
            wrap_column: u16::try_from(value.wrap_column)?,
            line_information: value.line_information,
            start_paused: value.start_paused,
            auto_pause: value.auto_pause,
            unpause_on_send: value.unpause_on_send,
            flash_taskbar_icon: value.flash_taskbar_icon,
            disable_compression: value.disable_compression,
            indent_paras: value.indent_paras,
            naws: value.naws,
            carriage_return_clears_line: value.carriage_return_clears_line,
            utf_8: value.utf_8,
            auto_wrap_window_width: value.auto_wrap_window_width,
            show_connect_disconnect: value.show_connect_disconnect,
            copy_selection_to_clipboard: value.copy_selection_to_clipboard,
            auto_copy_to_clipboard_in_html: value.auto_copy_to_clipboard_in_html,
            convert_ga_to_newline: value.convert_ga_to_newline,
            terminal_identification: String::from(&value.terminal_identification),
            use_mxp: value.use_mxp.try_into()?,
            hyperlink_colour: value.hyperlink_colour.convert(),
            use_custom_link_colour: value.use_custom_link_colour,
            mud_can_change_link_colour: value.mud_can_change_link_colour,
            underline_hyperlinks: value.underline_hyperlinks,
            mud_can_remove_underline: value.mud_can_remove_underline,
            hyperlink_adds_to_command_history: value.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: value.echo_hyperlink_in_output_window,
            ignore_mxp_colour_changes: value.ignore_mxp_colour_changes,
            send_mxp_afk_response: value.send_mxp_afk_response,
            ansi_colors: (&value.ansi_colors).into(),
            use_default_colours: value.use_default_colours,
            custom_color: value.custom_color.convert(),
            error_colour: value.error_colour.convert(),
            triggers: value.triggers.clone(),
            enable_triggers: value.enable_triggers,
            enable_trigger_sounds: value.enable_trigger_sounds,
            treeview_triggers: value.treeview_triggers,
            display_my_input: value.display_my_input,
            echo_colors: ColorPair {
                foreground: value.echo_text_colour.convert(),
                background: value.echo_background_colour.convert(),
            },
            enable_speed_walk: value.enable_speed_walk,
            speed_walk_prefix: String::from(&value.speed_walk_prefix),
            speed_walk_filler: String::from(&value.speed_walk_filler),
            speed_walk_delay: value.speed_walk_delay,
            enable_command_stack: value.enable_command_stack,
            command_stack_character: String::from(&value.command_stack_character),
            input_colors: ColorPair {
                foreground: value.input_text_colour.convert(),
                background: value.input_background_colour.convert(),
            },
            input_font: String::from(&value.input_font),
            input_font_height: u8::try_from(value.input_font_height)?,
            use_default_input_font: value.use_default_input_font,
            enable_spam_prevention: value.enable_spam_prevention,
            spam_line_count: usize::try_from(value.spam_line_count)?,
            spam_message: String::from(&value.spam_message),
            auto_repeat: value.auto_repeat,
            lower_case_tab_completion: value.lower_case_tab_completion,
            translate_german: value.translate_german,
            translate_backslash_sequences: value.translate_backslash_sequences,
            keep_commands_on_same_line: value.keep_commands_on_same_line,
            no_echo_off: value.no_echo_off,
            tab_completion_lines: usize::try_from(value.tab_completion_lines)?,
            tab_completion_space: value.tab_completion_space,
            double_click_inserts: value.double_click_inserts,
            double_click_sends: value.double_click_sends,
            escape_deletes_input: value.escape_deletes_input,
            save_deleted_command: value.save_deleted_command,
            confirm_before_replacing_typing: value.confirm_before_replacing_typing,
            arrow_keys_wrap: value.arrow_keys_wrap,
            arrows_change_history: value.arrows_change_history,
            arrow_recalls_partial: value.arrow_recalls_partial,
            alt_arrow_recalls_partial: value.alt_arrow_recalls_partial,
            ctrl_z_goes_to_end_of_buffer: value.ctrl_z_goes_to_end_of_buffer,
            ctrl_p_goes_to_previous_command: value.ctrl_p_goes_to_previous_command,
            ctrl_n_goes_to_next_command: value.ctrl_n_goes_to_next_command,
            history_lines: usize::try_from(value.history_lines)?,
            aliases: value.aliases.clone(),
            enable_aliases: value.enable_aliases,
            treeview_aliases: value.treeview_aliases,
            keypad_enable: value.keypad_enable,
            keypad_shortcuts: value.keypad_shortcuts.clone(),
            enable_auto_say: value.enable_auto_say,
            autosay_exclude_non_alpha: value.autosay_exclude_non_alpha,
            autosay_exclude_macros: value.autosay_exclude_macros,
            auto_say_override_prefix: String::from(&value.auto_say_override_prefix),
            auto_say_string: String::from(&value.auto_say_string),
            re_evaluate_auto_say: value.re_evaluate_auto_say,
            paste_line_preamble: String::from(&value.paste_line_preamble),
            paste_line_postamble: String::from(&value.paste_line_postamble),
            paste_delay: u32::try_from(value.paste_delay)?,
            paste_delay_per_lines: u32::try_from(value.paste_delay_per_lines)?,
            paste_commented_softcode: value.paste_commented_softcode,
            paste_echo: value.paste_echo,
            confirm_on_paste: value.confirm_on_paste,
            send_line_preamble: String::from(&value.send_line_preamble),
            send_line_postamble: String::from(&value.send_line_postamble),
            send_file_delay: u32::try_from(value.send_file_delay)?,
            send_file_delay_per_lines: u32::try_from(value.send_file_delay_per_lines)?,
            send_commented_softcode: value.send_commented_softcode,
            send_echo: value.send_echo,
            confirm_on_send: value.confirm_on_send,
            world_script: String::from(&value.world_script),
            script_prefix: String::from(&value.script_prefix),
            enable_scripts: value.enable_scripts,
            warn_if_scripting_inactive: value.warn_if_scripting_inactive,
            edit_script_with_notepad: value.edit_script_with_notepad,
            script_editor: String::from(&value.script_editor),
            script_reload_option: value.script_reload_option.try_into()?,
            script_errors_to_output_window: value.script_errors_to_output_window,
            note_text_colour: value.note_text_colour.convert(),
            plugins: Vec::new(),
        };
        world.aliases.sort_unstable();
        world.timers.sort_unstable();
        world.triggers.sort_unstable();
        Ok(world)
    }
}

trait IntoResultString {
    fn result(self) -> QString;
}

impl<E: Error> IntoResultString for Result<(), E> {
    fn result(self) -> QString {
        match self {
            Ok(()) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        }
    }
}

impl<E: Error> IntoResultString for Result<String, E> {
    fn result(self) -> QString {
        match self {
            Ok(s) => QString::from(&s),
            Err(e) => QString::from(&e.to_string()),
        }
    }
}

impl ffi::World {
    pub fn add_alias(self: Pin<&mut Self>, alias: &ffi::Alias) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .add_alias(alias.cxx_qt_ffi_rust())
            .result()
    }

    pub fn add_timer(self: Pin<&mut Self>, timer: &ffi::Timer) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .add_timer(timer.cxx_qt_ffi_rust());
        QString::default()
    }

    pub fn add_trigger(self: Pin<&mut Self>, trigger: &ffi::Trigger) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .add_trigger(trigger.cxx_qt_ffi_rust())
            .result()
    }

    pub fn export_aliases(&self) -> QString {
        self.cxx_qt_ffi_rust().export_aliases().result()
    }

    pub fn export_timers(&self) -> QString {
        self.cxx_qt_ffi_rust().export_timers().result()
    }

    pub fn export_triggers(&self) -> QString {
        self.cxx_qt_ffi_rust().export_triggers().result()
    }

    pub fn import_aliases(self: Pin<&mut Self>, xml: &QString) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .import_aliases(&String::from(xml))
            .result()
    }

    pub fn import_timers(self: Pin<&mut Self>, xml: &QString) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .import_timers(&String::from(xml))
            .result()
    }

    pub fn import_triggers(self: Pin<&mut Self>, xml: &QString) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .import_triggers(&String::from(xml))
            .result()
    }

    pub fn num_aliases(&self) -> usize {
        self.cxx_qt_ffi_rust().aliases.len()
    }

    pub fn num_timers(&self) -> usize {
        self.cxx_qt_ffi_rust().timers.len()
    }

    pub fn num_triggers(&self) -> usize {
        self.cxx_qt_ffi_rust().triggers.len()
    }

    pub fn remove_alias(self: Pin<&mut Self>, index: usize) {
        self.cxx_qt_ffi_rust_mut().remove_alias(index);
    }

    pub fn remove_timer(self: Pin<&mut Self>, index: usize) {
        self.cxx_qt_ffi_rust_mut().remove_timer(index);
    }

    pub fn remove_trigger(self: Pin<&mut Self>, index: usize) {
        self.cxx_qt_ffi_rust_mut().remove_trigger(index);
    }

    pub fn replace_alias(self: Pin<&mut Self>, index: usize, alias: &ffi::Alias) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .replace_alias(index, alias.cxx_qt_ffi_rust())
            .result()
    }

    pub fn replace_timer(self: Pin<&mut Self>, index: usize, timer: &ffi::Timer) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .replace_timer(index, timer.cxx_qt_ffi_rust());
        QString::default()
    }

    pub fn replace_trigger(self: Pin<&mut Self>, index: usize, trigger: &ffi::Trigger) -> QString {
        self.cxx_qt_ffi_rust_mut()
            .replace_trigger(index, trigger.cxx_qt_ffi_rust())
            .result()
    }

    pub fn build_alias_tree(&self, builder: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust().build_alias_tree(&mut builder.into());
    }

    pub fn build_timer_tree(&self, builder: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust().build_timer_tree(&mut builder.into());
    }

    pub fn build_trigger_tree(&self, builder: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust()
            .build_trigger_tree(&mut builder.into());
    }
}
