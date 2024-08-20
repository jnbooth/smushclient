#![allow(clippy::cast_possible_wrap)]
#![allow(clippy::cast_sign_loss)]
#![allow(clippy::missing_safety_doc)]
#![allow(clippy::float_cmp)]
#![allow(clippy::unnecessary_box_returns)]

#[macro_use]
mod convert;

mod client;
use client::SmushClientRust;

mod handler;

mod impls;

mod output;
use output::{RustOutputFragment, RustTelnetFragment, RustTextFragment};

mod sync;

mod world;
use world::WorldRust;

#[cxx_qt::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("cxx-qt-lib/qbytearray.h");
        type QByteArray = cxx_qt_lib::QByteArray;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
    }

    extern "C++Qt" {
        include!(<QtCore/QIODevice>);
        type QIODevice;

        unsafe fn read(self: Pin<&mut QIODevice>, data: *mut c_char, max_size: i64) -> i64;
        unsafe fn write(self: Pin<&mut QIODevice>, data: *const c_char, max_size: i64) -> i64;
    }

    extern "RustQt" {
        #[qobject]
        type SmushClient = super::SmushClientRust;
    }

    unsafe extern "RustQt" {
        fn read(self: Pin<&mut SmushClient>, device: Pin<&mut QIODevice>) -> i64;
        fn try_next(self: Pin<&mut SmushClient>) -> bool;
        fn next(self: &SmushClient) -> &RustOutputFragment;
    }

    unsafe impl !cxx_qt::Locking for SmushClient {}

    enum Heading {
        H1,
        H2,
        H3,
        H4,
        H5,
        H6,
        Normal,
    }

    enum EffectFragment {
        Backspace,
        Beep,
        CarriageReturn,
        EraseCharacter,
        EraseLine,
    }

    enum SendTo {
        World,
        Input,
        Internet,
    }

    struct MxpLink {
        action: QString,
        hint: QString,
        prompts: QStringList,
        sendto: SendTo,
    }

    struct MxpEntitySet {
        name: QString,
        value: QString,
        publish: bool,
        is_variable: bool,
    }

    struct MxpEntityUnset {
        name: QString,
        is_variable: bool,
    }

    extern "Rust" {
        type RustTextFragment;
        fn text(&self) -> QString;
        fn foreground(&self) -> QColor;
        fn background(&self) -> QColor;
        fn is_blink(&self) -> bool;
        fn is_bold(&self) -> bool;
        fn is_highlight(&self) -> bool;
        fn is_inverse(&self) -> bool;
        fn is_italic(&self) -> bool;
        fn is_strikeout(&self) -> bool;
        fn is_underline(&self) -> bool;
        fn has_link(&self) -> bool;
        fn link(&self) -> MxpLink;
        fn has_font(&self) -> bool;
        fn font(&self) -> QString;
        fn has_size(&self) -> bool;
        fn size(&self) -> u8;
        fn is_heading(&self) -> bool;
        fn heading(&self) -> Heading;
    }

    enum TelnetRequest {
        Afk,
        Do,
        IacGa,
        Naws,
        Subnegotiation,
        Will,
    }

    extern "Rust" {
        type RustTelnetFragment;
        fn kind(&self) -> TelnetRequest;
        fn code(&self) -> u8;
        fn data(&self) -> QByteArray;
    }

    #[derive(Debug)]
    enum OutputKind {
        Effect,
        Hr,
        LineBreak,
        MxpError,
        MxpEntitySet,
        MxpEntityUnset,
        PageBreak,
        Telnet,
        Text,
    }

    extern "Rust" {
        type RustOutputFragment;
        fn kind(&self) -> OutputKind;
        fn mxp_entity_set(&self) -> MxpEntitySet;
        fn mxp_entity_unset(&self) -> MxpEntityUnset;
        fn mxp_error(&self) -> QString;
        fn telnet(&self) -> &RustTelnetFragment;
        fn text(&self) -> &RustTextFragment;
    }

    extern "RustQt" {
        #[qobject]
        // IP address
        #[qproperty(QString, name)]
        #[qproperty(QString, site)]
        #[qproperty(u16, port)]
        #[qproperty(ProxyType, proxy_type)]
        #[qproperty(QString, proxy_server)]
        #[qproperty(u16, proxy_port)]
        #[qproperty(QString, proxy_username)]
        #[qproperty(QString, proxy_password)]
        #[qproperty(bool, proxy_password_base64)]
        #[qproperty(bool, save_world_automatically)]
        // Connecting
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
        #[qproperty(bool, treeview_timers)]
        // Chat
        #[qproperty(QString, chat_name)]
        #[qproperty(bool, auto_allow_snooping)]
        #[qproperty(bool, accept_chat_connections)]
        #[qproperty(u16, chat_port)]
        #[qproperty(bool, validate_incoming_chat_calls)]
        #[qproperty(QColor, chat_color_fg)]
        #[qproperty(QColor, chat_color_bg)]
        #[qproperty(bool, ignore_chat_colors)]
        #[qproperty(QString, chat_message_prefix)]
        #[qproperty(usize, chat_max_lines_per_message)]
        #[qproperty(usize, chat_max_bytes_per_message)]
        #[qproperty(bool, auto_allow_files)]
        #[qproperty(QString, chat_file_save_directory)]
        // Notes
        #[qproperty(QString, notes)]
        // Output
        #[qproperty(QString, beep_sound)]
        #[qproperty(i16, pixel_offset)]
        #[qproperty(f32, line_spacing)]
        #[qproperty(QString, output_font)]
        #[qproperty(bool, use_default_output_font)]
        #[qproperty(bool, show_bold)]
        #[qproperty(bool, show_italic)]
        #[qproperty(bool, show_underline)]
        #[qproperty(QString, new_activity_sound)]
        #[qproperty(usize, max_output_lines)]
        #[qproperty(u16, wrap_column)]
        #[qproperty(bool, line_information)]
        #[qproperty(bool, start_paused)]
        #[qproperty(bool, auto_pause)]
        #[qproperty(bool, unpause_on_send)]
        #[qproperty(bool, flash_taskbar_icon)]
        #[qproperty(bool, disable_compression)]
        #[qproperty(bool, indent_paras)]
        #[qproperty(bool, naws)]
        #[qproperty(bool, carriage_return_clears_line)]
        #[qproperty(bool, utf_8)]
        #[qproperty(bool, auto_wrap_window_width)]
        #[qproperty(bool, show_connect_disconnect)]
        #[qproperty(bool, copy_selection_to_clipboard)]
        #[qproperty(bool, auto_copy_to_clipboard_in_html)]
        #[qproperty(bool, convert_ga_to_newline)]
        #[qproperty(QString, terminal_identification)]
        // MXP / Pueblo
        #[qproperty(UseMxp, use_mxp)]
        #[qproperty(bool, detect_pueblo)]
        #[qproperty(QColor, hyperlink_color)]
        #[qproperty(bool, use_custom_link_color)]
        #[qproperty(bool, mud_can_change_link_color)]
        #[qproperty(bool, underline_hyperlinks)]
        #[qproperty(bool, mud_can_remove_underline)]
        #[qproperty(bool, hyperlink_adds_to_command_history)]
        #[qproperty(bool, echo_hyperlink_in_output_window)]
        #[qproperty(bool, ignore_mxp_color_changes)]
        #[qproperty(bool, send_mxp_afk_response)]
        #[qproperty(bool, mud_can_change_options)]
        // ANSI Color
        #[qproperty(bool, use_default_colors)]
        // Custom Color
        #[qproperty(QStringList, custom_names)]
        // Triggers
        #[qproperty(bool, enable_triggers)]
        #[qproperty(bool, enable_trigger_sounds)]
        #[qproperty(bool, treeview_triggers)]
        // Commands
        #[qproperty(bool, display_my_input)]
        #[qproperty(QColor, echo_color_fg)]
        #[qproperty(QColor, echo_color_bg)]
        #[qproperty(bool, enable_speed_walk)]
        #[qproperty(QString, speed_walk_prefix)]
        #[qproperty(QString, speed_walk_filler)]
        #[qproperty(u32, speed_walk_delay)]
        #[qproperty(bool, enable_command_stack)]
        #[qproperty(QString, command_stack_character)]
        #[qproperty(QColor, input_color_fg)]
        #[qproperty(QColor, input_color_bg)]
        #[qproperty(QString, input_font)]
        #[qproperty(bool, use_default_input_font)]
        #[qproperty(bool, enable_spam_prevention)]
        #[qproperty(usize, spam_line_count)]
        #[qproperty(QString, spam_message)]
        #[qproperty(bool, auto_repeat)]
        #[qproperty(bool, lower_case_tab_completion)]
        #[qproperty(bool, translate_german)]
        #[qproperty(bool, translate_backslash_sequences)]
        #[qproperty(bool, keep_commands_on_same_line)]
        #[qproperty(bool, no_echo_off)]
        #[qproperty(usize, tab_completion_lines)]
        #[qproperty(bool, tab_completion_space)]
        #[qproperty(bool, double_click_inserts)]
        #[qproperty(bool, double_click_sends)]
        #[qproperty(bool, escape_deletes_input)]
        #[qproperty(bool, save_deleted_command)]
        #[qproperty(bool, confirm_before_replacing_typing)]
        #[qproperty(bool, arrow_keys_wrap)]
        #[qproperty(bool, arrows_change_history)]
        #[qproperty(bool, arrow_recalls_partial)]
        #[qproperty(bool, alt_arrow_recalls_partial)]
        #[qproperty(bool, ctrl_z_goes_to_end_of_buffer)]
        #[qproperty(bool, ctrl_p_goes_to_previous_command)]
        #[qproperty(bool, ctrl_n_goes_to_next_command)]
        #[qproperty(usize, history_lines)]
        // Aliases
        #[qproperty(bool, enable_aliases)]
        #[qproperty(bool, treeview_aliases)]
        // Keypad
        #[qproperty(bool, keypad_enable)]
        // Auto Say
        #[qproperty(bool, enable_auto_say)]
        #[qproperty(bool, autosay_exclude_non_alpha)]
        #[qproperty(bool, autosay_exclude_macros)]
        #[qproperty(QString, auto_say_override_prefix)]
        #[qproperty(QString, auto_say_string)]
        #[qproperty(bool, re_evaluate_auto_say)]
        // Paste
        #[qproperty(QString, paste_line_preamble)]
        #[qproperty(QString, paste_line_postamble)]
        #[qproperty(u32, paste_delay)]
        #[qproperty(u32, paste_delay_per_lines)]
        #[qproperty(bool, paste_commented_softcode)]
        #[qproperty(bool, paste_echo)]
        #[qproperty(bool, confirm_on_paste)]
        // Send
        #[qproperty(QString, send_line_preamble)]
        #[qproperty(QString, send_line_postamble)]
        #[qproperty(u32, send_delay)]
        #[qproperty(u32, send_delay_per_lines)]
        #[qproperty(bool, send_commented_softcode)]
        #[qproperty(bool, send_echo)]
        #[qproperty(bool, confirm_on_send)]
        // Scripts
        #[qproperty(QString, world_script)]
        #[qproperty(QString, script_prefix)]
        #[qproperty(bool, enable_scripts)]
        #[qproperty(bool, warn_if_scripting_inactive)]
        #[qproperty(bool, edit_script_with_notepad)]
        #[qproperty(QString, script_editor)]
        #[qproperty(ScriptRecompile, script_reload_option)]
        #[qproperty(bool, script_errors_to_output_window)]
        #[qproperty(QColor, note_text_color)]
        type World = super::WorldRust;
    }

    unsafe extern "RustQt" {
        fn populate(self: Pin<&mut World>, client: &SmushClient);
    }

    enum ProxyType {
        None,
        Socks4,
        Socks5,
    }

    enum AutoConnect {
        None,
        Mush,
        Diku,
        Mxp,
    }

    enum ScriptRecompile {
        Confirm,
        Always,
        Never,
    }

    enum LogFormat {
        Text,
        Html,
        Raw,
    }

    enum LogMode {
        Append,
        Overwrite,
    }

    enum UseMxp {
        Command,
        Query,
        Always,
        Never,
    }
}
