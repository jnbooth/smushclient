use std::collections::HashMap;
use std::ffi::{c_double, c_int};
use std::ops::{Deref, DerefMut};

use crate::colors::Colors;
use crate::convert::Convert;
use cxx_qt_lib::{QColor, QString, QStringList};
use mud_transformer::mxp::RgbColor;
use smushclient::world::ColorPair;
use smushclient::World;
use smushclient_plugins::{Alias, Timer, Trigger};

use crate::ffi;

#[derive(Default)]
pub struct WorldRust {
    colors: Colors,

    // IP address
    pub name: QString,
    pub site: QString,
    pub port: c_int,
    pub proxy_type: ffi::ProxyType,
    pub proxy_server: QString,
    pub proxy_port: c_int,
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
    pub chat_port: c_int,
    pub validate_incoming_chat_calls: bool,
    pub chat_colors_foreground: QColor,
    pub chat_colors_background: QColor,
    pub ignore_chat_colors: bool,
    pub chat_message_prefix: QString,
    pub chat_max_lines_per_message: c_int,
    pub chat_max_bytes_per_message: c_int,
    pub auto_allow_files: bool,
    pub chat_file_save_directory: QString,

    // Notes
    pub notes: QString,

    // Output
    pub beep_sound: QString,
    pub pixel_offset: c_int,
    pub line_spacing: c_double,
    pub output_font: QString,
    pub output_font_size: c_int,
    pub use_default_output_font: bool,
    pub show_bold: bool,
    pub show_italic: bool,
    pub show_underline: bool,
    pub new_activity_sound: QString,
    pub max_output_lines: c_int,
    pub wrap_column: c_int,

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
    pub hyperlink_color: QColor,
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

    // Triggers
    pub triggers: Vec<Trigger>,
    pub enable_triggers: bool,
    pub enable_trigger_sounds: bool,
    pub treeview_triggers: bool,

    // Commands
    pub display_my_input: bool,
    pub echo_colors_foreground: QColor,
    pub echo_colors_background: QColor,
    pub enable_speed_walk: bool,
    pub speed_walk_prefix: QString,
    pub speed_walk_filler: QString,
    pub speed_walk_delay: c_double,
    pub enable_command_stack: bool,
    pub command_stack_character: QString,
    pub input_colors_foreground: QColor,
    pub input_colors_background: QColor,
    pub input_font: QString,
    pub input_font_size: c_int,
    pub use_default_input_font: bool,
    pub enable_spam_prevention: bool,
    pub spam_line_count: c_int,
    pub spam_message: QString,

    pub auto_repeat: bool,
    pub lower_case_tab_completion: bool,
    pub translate_german: bool,
    pub translate_backslash_sequences: bool,
    pub keep_commands_on_same_line: bool,
    pub no_echo_off: bool,
    pub tab_completion_lines: c_int,
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
    pub history_lines: c_int,

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
    pub paste_delay: c_int,
    pub paste_delay_per_lines: c_int,
    pub paste_commented_softcode: bool,
    pub paste_echo: bool,
    pub confirm_on_paste: bool,

    // Send
    pub send_line_preamble: QString,
    pub send_line_postamble: QString,
    pub send_delay: c_int,
    pub send_delay_per_lines: c_int,
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

impl Deref for WorldRust {
    type Target = Colors;

    fn deref(&self) -> &Self::Target {
        &self.colors
    }
}

impl DerefMut for WorldRust {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.colors
    }
}

#[allow(clippy::useless_conversion)]
impl From<&WorldRust> for World {
    fn from(value: &WorldRust) -> Self {
        let mut ansi_colors = <[RgbColor; 16]>::default();
        let mut custom_names = <[String; 16]>::default();
        let mut custom_colors = <[ColorPair; 16]>::default();
        let iter = value
            .colors
            .array()
            .into_iter()
            .zip(ansi_colors.iter_mut())
            .zip(custom_names.iter_mut())
            .zip(custom_colors.iter_mut());

        for (((entry, ansi_color), custom_name), custom_color) in iter {
            *ansi_color = entry.ansi_foreground.convert();
            *custom_name = String::from(entry.custom_name);
            *custom_color = ColorPair {
                foreground: entry.custom_foreground.convert(),
                background: entry.custom_background.convert(),
            }
        }

        Self {
            name: String::from(&value.name),
            site: String::from(&value.site),
            port: u16::try_from(value.port).unwrap(),
            proxy_type: value.proxy_type.into(),
            proxy_server: String::from(&value.proxy_server),
            proxy_port: u16::try_from(value.proxy_port).unwrap(),
            proxy_username: String::from(&value.proxy_username),
            proxy_password: String::from(&value.proxy_password),
            proxy_password_base64: value.proxy_password_base64,
            save_world_automatically: value.save_world_automatically,
            player: String::from(&value.player),
            password: String::from(&value.password),
            connect_method: value.connect_method.into(),
            connect_text: String::from(&value.connect_text),
            log_file_preamble: String::from(&value.log_file_preamble),
            log_file_postamble: String::from(&value.log_file_postamble),
            log_format: value.log_format.into(),
            log_output: value.log_output,
            log_input: value.log_input,
            log_notes: value.log_notes,
            log_mode: value.log_mode.into(),
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
            chat_port: u16::try_from(value.chat_port).unwrap(),
            validate_incoming_chat_calls: value.validate_incoming_chat_calls,
            chat_colors: ColorPair {
                foreground: value.chat_colors_foreground.convert(),
                background: value.chat_colors_background.convert(),
            },
            ignore_chat_colors: value.ignore_chat_colors,
            chat_message_prefix: String::from(&value.chat_message_prefix),
            chat_max_lines_per_message: usize::try_from(value.chat_max_lines_per_message).unwrap(),
            chat_max_bytes_per_message: usize::try_from(value.chat_max_bytes_per_message).unwrap(),
            auto_allow_files: value.auto_allow_files,
            chat_file_save_directory: value.chat_file_save_directory.convert(),
            notes: String::from(&value.notes),
            beep_sound: value.beep_sound.convert(),
            pixel_offset: i16::try_from(value.pixel_offset).unwrap(),
            line_spacing: f64::try_from(value.line_spacing).unwrap(),
            output_font: String::from(&value.output_font),
            output_font_size: u8::try_from(value.output_font_size).unwrap(),
            use_default_output_font: value.use_default_output_font,
            show_bold: value.show_bold,
            show_italic: value.show_italic,
            show_underline: value.show_underline,
            new_activity_sound: value.new_activity_sound.convert(),
            max_output_lines: usize::try_from(value.max_output_lines).unwrap(),
            wrap_column: u16::try_from(value.wrap_column).unwrap(),
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
            use_mxp: value.use_mxp.into(),
            hyperlink_color: value.hyperlink_color.convert(),
            use_custom_link_color: value.use_custom_link_color,
            mud_can_change_link_color: value.mud_can_change_link_color,
            underline_hyperlinks: value.underline_hyperlinks,
            mud_can_remove_underline: value.mud_can_remove_underline,
            hyperlink_adds_to_command_history: value.hyperlink_adds_to_command_history,
            echo_hyperlink_in_output_window: value.echo_hyperlink_in_output_window,
            ignore_mxp_color_changes: value.ignore_mxp_color_changes,
            send_mxp_afk_response: value.send_mxp_afk_response,
            use_default_colors: value.use_default_colors,
            ansi_colors,
            custom_names,
            custom_colors,
            triggers: value.triggers.clone(),
            enable_triggers: value.enable_triggers,
            enable_trigger_sounds: value.enable_trigger_sounds,
            treeview_triggers: value.treeview_triggers,
            display_my_input: value.display_my_input,
            echo_colors: ColorPair {
                foreground: value.echo_colors_foreground.convert(),
                background: value.echo_colors_background.convert(),
            },
            enable_speed_walk: value.enable_speed_walk,
            speed_walk_prefix: String::from(&value.speed_walk_prefix),
            speed_walk_filler: String::from(&value.speed_walk_filler),
            speed_walk_delay: f64::try_from(value.speed_walk_delay).unwrap(),
            enable_command_stack: value.enable_command_stack,
            command_stack_character: String::from(&value.command_stack_character),
            input_colors: ColorPair {
                foreground: value.input_colors_foreground.convert(),
                background: value.input_colors_background.convert(),
            },
            input_font: String::from(&value.input_font),
            input_font_size: u8::try_from(value.input_font_size).unwrap(),
            use_default_input_font: value.use_default_input_font,
            enable_spam_prevention: value.enable_spam_prevention,
            spam_line_count: usize::try_from(value.spam_line_count).unwrap(),
            spam_message: String::from(&value.spam_message),
            auto_repeat: value.auto_repeat,
            lower_case_tab_completion: value.lower_case_tab_completion,
            translate_german: value.translate_german,
            translate_backslash_sequences: value.translate_backslash_sequences,
            keep_commands_on_same_line: value.keep_commands_on_same_line,
            no_echo_off: value.no_echo_off,
            tab_completion_lines: usize::try_from(value.tab_completion_lines).unwrap(),
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
            history_lines: usize::try_from(value.history_lines).unwrap(),
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
            paste_delay: u32::try_from(value.paste_delay).unwrap(),
            paste_delay_per_lines: u32::try_from(value.paste_delay_per_lines).unwrap(),
            paste_commented_softcode: value.paste_commented_softcode,
            paste_echo: value.paste_echo,
            confirm_on_paste: value.confirm_on_paste,
            send_line_preamble: String::from(&value.send_line_preamble),
            send_line_postamble: String::from(&value.send_line_postamble),
            send_delay: u32::try_from(value.send_delay).unwrap(),
            send_delay_per_lines: u32::try_from(value.send_delay_per_lines).unwrap(),
            send_commented_softcode: value.send_commented_softcode,
            send_echo: value.send_echo,
            confirm_on_send: value.confirm_on_send,
            world_script: String::from(&value.world_script),
            script_prefix: String::from(&value.script_prefix),
            enable_scripts: value.enable_scripts,
            warn_if_scripting_inactive: value.warn_if_scripting_inactive,
            edit_script_with_notepad: value.edit_script_with_notepad,
            script_editor: String::from(&value.script_editor),
            script_reload_option: value.script_reload_option.into(),
            script_errors_to_output_window: value.script_errors_to_output_window,
            note_text_color: value.note_text_color.convert(),
            plugins: value.plugins.convert(),
        }
    }
}

impl WorldRust {
    #[allow(clippy::unnecessary_fallible_conversions)]
    pub fn populate(&mut self, world: &World) {
        let iter = self
            .colors
            .array_mut()
            .into_iter()
            .zip(world.ansi_colors.iter())
            .zip(world.custom_names.iter())
            .zip(world.custom_colors.iter());

        for (((entry, ansi_color), custom_name), custom_color) in iter {
            *entry.ansi_foreground = ansi_color.convert();
            *entry.custom_name = QString::from(custom_name);
            *entry.custom_foreground = custom_color.foreground.convert();
            *entry.custom_background = custom_color.background.convert();
        }

        self.name = QString::from(&world.name);
        self.site = QString::from(&world.site);
        self.port = c_int::from(world.port);
        self.proxy_type = world.proxy_type.into();
        self.proxy_server = QString::from(&world.proxy_server);
        self.proxy_port = c_int::from(world.proxy_port);
        self.proxy_username = QString::from(&world.proxy_username);
        self.proxy_password = QString::from(&world.proxy_password);
        self.proxy_password_base64 = world.proxy_password_base64;
        self.save_world_automatically = world.save_world_automatically;
        self.player = QString::from(&world.player);
        self.password = QString::from(&world.password);
        self.connect_method = world.connect_method.into();
        self.connect_text = QString::from(&world.connect_text);
        self.log_file_preamble = QString::from(&world.log_file_preamble);
        self.log_file_postamble = QString::from(&world.log_file_postamble);
        self.log_format = world.log_format.into();
        self.log_output = world.log_output;
        self.log_input = world.log_input;
        self.log_notes = world.log_notes;
        self.log_mode = world.log_mode.into();
        self.auto_log_file_name = world.auto_log_file_name.convert();
        self.log_preamble_output = QString::from(&world.log_preamble_output);
        self.log_preamble_input = QString::from(&world.log_preamble_input);
        self.log_preamble_notes = QString::from(&world.log_preamble_notes);
        self.log_postamble_output = QString::from(&world.log_postamble_output);
        self.log_postamble_input = QString::from(&world.log_postamble_input);
        self.log_postamble_notes = QString::from(&world.log_postamble_notes);
        self.timers = world.timers.clone();
        self.enable_timers = world.enable_timers;
        self.treeview_timers = world.treeview_timers;
        self.chat_name = QString::from(&world.chat_name);
        self.auto_allow_snooping = world.auto_allow_snooping;
        self.accept_chat_connections = world.accept_chat_connections;
        self.chat_port = c_int::from(world.chat_port);
        self.validate_incoming_chat_calls = world.validate_incoming_chat_calls;
        self.chat_colors_foreground = world.chat_colors.foreground.convert();
        self.chat_colors_background = world.chat_colors.background.convert();
        self.ignore_chat_colors = world.ignore_chat_colors;
        self.chat_message_prefix = QString::from(&world.chat_message_prefix);
        self.chat_max_lines_per_message =
            c_int::try_from(world.chat_max_lines_per_message).unwrap();
        self.chat_max_bytes_per_message =
            c_int::try_from(world.chat_max_bytes_per_message).unwrap();
        self.auto_allow_files = world.auto_allow_files;
        self.chat_file_save_directory = world.chat_file_save_directory.convert();
        self.notes = QString::from(&world.notes);
        self.beep_sound = world.beep_sound.convert();
        self.pixel_offset = c_int::from(world.pixel_offset);
        self.line_spacing = c_double::try_from(world.line_spacing).unwrap();
        self.output_font = QString::from(&world.output_font);
        self.output_font_size = c_int::from(world.output_font_size);
        self.use_default_output_font = world.use_default_output_font;
        self.show_bold = world.show_bold;
        self.show_italic = world.show_italic;
        self.show_underline = world.show_underline;
        self.new_activity_sound = world.new_activity_sound.convert();
        self.max_output_lines = c_int::try_from(world.max_output_lines).unwrap();
        self.wrap_column = c_int::from(world.wrap_column);
        self.line_information = world.line_information;
        self.start_paused = world.start_paused;
        self.auto_pause = world.auto_pause;
        self.unpause_on_send = world.unpause_on_send;
        self.flash_taskbar_icon = world.flash_taskbar_icon;
        self.disable_compression = world.disable_compression;
        self.indent_paras = world.indent_paras;
        self.naws = world.naws;
        self.carriage_return_clears_line = world.carriage_return_clears_line;
        self.utf_8 = world.utf_8;
        self.auto_wrap_window_width = world.auto_wrap_window_width;
        self.show_connect_disconnect = world.show_connect_disconnect;
        self.copy_selection_to_clipboard = world.copy_selection_to_clipboard;
        self.auto_copy_to_clipboard_in_html = world.auto_copy_to_clipboard_in_html;
        self.convert_ga_to_newline = world.convert_ga_to_newline;
        self.terminal_identification = QString::from(&world.terminal_identification);
        self.use_mxp = world.use_mxp.into();
        self.hyperlink_color = world.hyperlink_color.convert();
        self.use_custom_link_color = world.use_custom_link_color;
        self.mud_can_change_link_color = world.mud_can_change_link_color;
        self.underline_hyperlinks = world.underline_hyperlinks;
        self.mud_can_remove_underline = world.mud_can_remove_underline;
        self.hyperlink_adds_to_command_history = world.hyperlink_adds_to_command_history;
        self.echo_hyperlink_in_output_window = world.echo_hyperlink_in_output_window;
        self.ignore_mxp_color_changes = world.ignore_mxp_color_changes;
        self.send_mxp_afk_response = world.send_mxp_afk_response;
        self.use_default_colors = world.use_default_colors;
        self.triggers = world.triggers.clone();
        self.enable_triggers = world.enable_triggers;
        self.enable_trigger_sounds = world.enable_trigger_sounds;
        self.treeview_triggers = world.treeview_triggers;
        self.display_my_input = world.display_my_input;
        self.echo_colors_foreground = world.echo_colors.foreground.convert();
        self.echo_colors_background = world.echo_colors.background.convert();
        self.enable_speed_walk = world.enable_speed_walk;
        self.speed_walk_prefix = QString::from(&world.speed_walk_prefix);
        self.speed_walk_filler = QString::from(&world.speed_walk_filler);
        self.speed_walk_delay = c_double::try_from(world.speed_walk_delay).unwrap();
        self.enable_command_stack = world.enable_command_stack;
        self.command_stack_character = QString::from(&world.command_stack_character);
        self.input_colors_foreground = world.input_colors.foreground.convert();
        self.input_colors_background = world.input_colors.background.convert();
        self.input_font = QString::from(&world.input_font);
        self.input_font_size = c_int::from(world.input_font_size);
        self.use_default_input_font = world.use_default_input_font;
        self.enable_spam_prevention = world.enable_spam_prevention;
        self.spam_line_count = c_int::try_from(world.spam_line_count).unwrap();
        self.spam_message = QString::from(&world.spam_message);
        self.auto_repeat = world.auto_repeat;
        self.lower_case_tab_completion = world.lower_case_tab_completion;
        self.translate_german = world.translate_german;
        self.translate_backslash_sequences = world.translate_backslash_sequences;
        self.keep_commands_on_same_line = world.keep_commands_on_same_line;
        self.no_echo_off = world.no_echo_off;
        self.tab_completion_lines = c_int::try_from(world.tab_completion_lines).unwrap();
        self.tab_completion_space = world.tab_completion_space;
        self.double_click_inserts = world.double_click_inserts;
        self.double_click_sends = world.double_click_sends;
        self.escape_deletes_input = world.escape_deletes_input;
        self.save_deleted_command = world.save_deleted_command;
        self.confirm_before_replacing_typing = world.confirm_before_replacing_typing;
        self.arrow_keys_wrap = world.arrow_keys_wrap;
        self.arrows_change_history = world.arrows_change_history;
        self.arrow_recalls_partial = world.arrow_recalls_partial;
        self.alt_arrow_recalls_partial = world.alt_arrow_recalls_partial;
        self.ctrl_z_goes_to_end_of_buffer = world.ctrl_z_goes_to_end_of_buffer;
        self.ctrl_p_goes_to_previous_command = world.ctrl_p_goes_to_previous_command;
        self.ctrl_n_goes_to_next_command = world.ctrl_n_goes_to_next_command;
        self.history_lines = c_int::try_from(world.history_lines).unwrap();
        self.aliases = world.aliases.clone();
        self.enable_aliases = world.enable_aliases;
        self.treeview_aliases = world.treeview_aliases;
        self.keypad_enable = world.keypad_enable;
        self.keypad_shortcuts = world.keypad_shortcuts.clone();
        self.enable_auto_say = world.enable_auto_say;
        self.autosay_exclude_non_alpha = world.autosay_exclude_non_alpha;
        self.autosay_exclude_macros = world.autosay_exclude_macros;
        self.auto_say_override_prefix = QString::from(&world.auto_say_override_prefix);
        self.auto_say_string = QString::from(&world.auto_say_string);
        self.re_evaluate_auto_say = world.re_evaluate_auto_say;
        self.paste_line_preamble = QString::from(&world.paste_line_preamble);
        self.paste_line_postamble = QString::from(&world.paste_line_postamble);
        self.paste_delay = c_int::try_from(world.paste_delay).unwrap();
        self.paste_delay_per_lines = c_int::try_from(world.paste_delay_per_lines).unwrap();
        self.paste_commented_softcode = world.paste_commented_softcode;
        self.paste_echo = world.paste_echo;
        self.confirm_on_paste = world.confirm_on_paste;
        self.send_line_preamble = QString::from(&world.send_line_preamble);
        self.send_line_postamble = QString::from(&world.send_line_postamble);
        self.send_delay = c_int::try_from(world.send_delay).unwrap();
        self.send_delay_per_lines = c_int::try_from(world.send_delay_per_lines).unwrap();
        self.send_commented_softcode = world.send_commented_softcode;
        self.send_echo = world.send_echo;
        self.confirm_on_send = world.confirm_on_send;
        self.world_script = QString::from(&world.world_script);
        self.script_prefix = QString::from(&world.script_prefix);
        self.enable_scripts = world.enable_scripts;
        self.warn_if_scripting_inactive = world.warn_if_scripting_inactive;
        self.edit_script_with_notepad = world.edit_script_with_notepad;
        self.script_editor = QString::from(&world.script_editor);
        self.script_reload_option = world.script_reload_option.into();
        self.script_errors_to_output_window = world.script_errors_to_output_window;
        self.note_text_color = world.note_text_color.convert();
        self.plugins = world.plugins.convert();
    }
}
