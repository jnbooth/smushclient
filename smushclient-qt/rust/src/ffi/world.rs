use crate::world::WorldRust;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    #[repr(i32)]
    enum AutoConnect {
        None,
        Mush,
        Diku,
        Mxp,
    }

    #[repr(i32)]
    enum LogFormat {
        Text,
        Html,
        Raw,
    }

    #[repr(i32)]
    enum LogMode {
        Append,
        Overwrite,
    }

    #[repr(i32)]
    enum UseMxp {
        Command,
        Query,
        Always,
        Never,
    }

    #[repr(i32)]
    enum ScriptRecompile {
        Confirm,
        Always,
        Never,
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        // Connecting
        #[qproperty(QString, name)]
        #[qproperty(QString, site)]
        #[qproperty(i32, port)]
        #[qproperty(bool, use_proxy)]
        #[qproperty(QString, proxy_server)]
        #[qproperty(i32, proxy_port)]
        #[qproperty(QString, proxy_username)]
        #[qproperty(QString, proxy_password)]
        #[qproperty(bool, save_world_automatically)]
        // Login
        #[qproperty(QString, player)]
        #[qproperty(QString, password)]
        #[qproperty(AutoConnect, connect_method)]
        #[qproperty(QString, connect_text)]
        // Logging
        #[qproperty(QString, log_file_preamble)]
        #[qproperty(QString, log_file_postamble)]
        #[qproperty(LogFormat, log_format)]
        #[qproperty(bool, log_output)]
        #[qproperty(bool, log_input)]
        #[qproperty(bool, log_notes)]
        #[qproperty(LogMode, log_mode)]
        #[qproperty(QString, auto_log_file_name)]
        #[qproperty(QString, log_preamble_output)]
        #[qproperty(QString, log_preamble_input)]
        #[qproperty(QString, log_preamble_notes)]
        #[qproperty(QString, log_postamble_output)]
        #[qproperty(QString, log_postamble_input)]
        #[qproperty(QString, log_postamble_notes)]
        // Timers
        #[qproperty(bool, enable_timers)]
        // Output
        #[qproperty(bool, show_bold)]
        #[qproperty(bool, show_italic)]
        #[qproperty(bool, show_underline)]
        #[qproperty(i32, indent_paras)]
        #[qproperty(QColor, ansi_0)]
        #[qproperty(QColor, ansi_1)]
        #[qproperty(QColor, ansi_2)]
        #[qproperty(QColor, ansi_3)]
        #[qproperty(QColor, ansi_4)]
        #[qproperty(QColor, ansi_5)]
        #[qproperty(QColor, ansi_6)]
        #[qproperty(QColor, ansi_7)]
        #[qproperty(QColor, ansi_8)]
        #[qproperty(QColor, ansi_9)]
        #[qproperty(QColor, ansi_10)]
        #[qproperty(QColor, ansi_11)]
        #[qproperty(QColor, ansi_12)]
        #[qproperty(QColor, ansi_13)]
        #[qproperty(QColor, ansi_14)]
        #[qproperty(QColor, ansi_15)]
        #[qproperty(bool, display_my_input)]
        #[qproperty(QColor, echo_text_colour)]
        #[qproperty(QColor, echo_background_colour)]
        #[qproperty(bool, keep_commands_on_same_line)]
        #[qproperty(QString, new_activity_sound)]
        // MUD
        #[qproperty(UseMxp, use_mxp)]
        #[qproperty(bool, ignore_mxp_colour_changes)]
        #[qproperty(bool, use_custom_link_colour)]
        #[qproperty(QColor, hyperlink_colour)]
        #[qproperty(bool, mud_can_change_link_colour)]
        #[qproperty(bool, underline_hyperlinks)]
        #[qproperty(bool, hyperlink_adds_to_command_history)]
        #[qproperty(bool, echo_hyperlink_in_output_window)]
        #[qproperty(bool, naws)]
        #[qproperty(bool, carriage_return_clears_line)]
        #[qproperty(bool, utf_8)]
        #[qproperty(bool, convert_ga_to_newline)]
        #[qproperty(QString, terminal_identification)]
        #[qproperty(bool, disable_compression)]
        #[qproperty(bool, no_echo_off)]
        #[qproperty(bool, enable_command_stack)]
        #[qproperty(u16, command_stack_character)]
        // Triggers
        #[qproperty(bool, enable_triggers)]
        // Aliases
        #[qproperty(bool, enable_aliases)]
        // Keypad
        #[qproperty(QString, numpad_0)]
        #[qproperty(QString, numpad_1)]
        #[qproperty(QString, numpad_2)]
        #[qproperty(QString, numpad_3)]
        #[qproperty(QString, numpad_4)]
        #[qproperty(QString, numpad_5)]
        #[qproperty(QString, numpad_6)]
        #[qproperty(QString, numpad_7)]
        #[qproperty(QString, numpad_8)]
        #[qproperty(QString, numpad_9)]
        #[qproperty(QString, numpad_period)]
        #[qproperty(QString, numpad_slash)]
        #[qproperty(QString, numpad_asterisk)]
        #[qproperty(QString, numpad_minus)]
        #[qproperty(QString, numpad_plus)]
        #[qproperty(QString, numpad_mod_0)]
        #[qproperty(QString, numpad_mod_1)]
        #[qproperty(QString, numpad_mod_2)]
        #[qproperty(QString, numpad_mod_3)]
        #[qproperty(QString, numpad_mod_4)]
        #[qproperty(QString, numpad_mod_5)]
        #[qproperty(QString, numpad_mod_6)]
        #[qproperty(QString, numpad_mod_7)]
        #[qproperty(QString, numpad_mod_8)]
        #[qproperty(QString, numpad_mod_9)]
        #[qproperty(QString, numpad_mod_period)]
        #[qproperty(QString, numpad_mod_slash)]
        #[qproperty(QString, numpad_mod_asterisk)]
        #[qproperty(QString, numpad_mod_minus)]
        #[qproperty(QString, numpad_mod_plus)]
        #[qproperty(bool, numpad_enable)]
        #[qproperty(bool, hotkey_adds_to_command_history)]
        #[qproperty(bool, echo_hotkey_in_output_window)]
        // Scripting
        #[qproperty(bool, enable_scripts)]
        #[qproperty(QString, world_script)]
        #[qproperty(ScriptRecompile, script_reload_option)]
        #[qproperty(QColor, note_text_colour)]
        #[qproperty(QColor, note_background_colour)]
        #[qproperty(QColor, error_text_colour)]
        #[qproperty(QColor, error_background_colour)]
        type World = super::WorldRust;
    }
}
