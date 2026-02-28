#![allow(dead_code)]
use crate::client::SmushClientRust;

#[cxx_qt::bridge]
pub mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;
        include!("cxx-qt-lib/qvector.h");
        type QVector_QColor = cxx_qt_lib::QVector<QColor>;

        include!("cxx-qt-io/qabstractsocket.h");
        type QAbstractSocket = cxx_qt_io::QAbstractSocket;

        include!("smushclient-qt-lib/qtextcharformat.h");
        type QTextCharFormat = smushclient_qt_lib::QTextCharFormat;
        include!("smushclient-qt-lib/qlist.h");
        type QList_QPair_QColor_QColor =
            cxx_qt_lib::QList<smushclient_qt_lib::QPair<QColor, QColor>>;
    }

    extern "C++" {
        include!("smushclient_qt/forward.h");
        type Alias = crate::ffi::Alias;
        type AliasOutcomes = crate::ffi::AliasOutcomes;
        type Document = crate::ffi::Document;
        type Timekeeper = crate::ffi::Timekeeper;
        type Timer = crate::ffi::Timer;
        type Trigger = crate::ffi::Trigger;
        type World = crate::ffi::World;

        include!("smushclient_qt/src/ffi/api_code.cxx.h");
        type ApiCode = crate::ffi::ApiCode;
        include!("smushclient_qt/src/ffi/regex.cxx.h");
        type RegexParse = crate::ffi::RegexParse;
        include!("smushclient_qt/src/ffi/send_request.cxx.h");
        type SenderKind = crate::ffi::SenderKind;
        type SendTimer = crate::ffi::SendTimer;
    }

    #[namespace = "rust"]
    extern "C++" {
        include!("smushclient_qt/views.h");
        #[cxx_name = "bytes_view"]
        type BytesView<'a> = crate::ffi::BytesView<'a>;
        #[cxx_name = "string_view"]
        type StringView<'a> = crate::ffi::StringView<'a>;
        #[cxx_name = "variable_view"]
        type VariableView = crate::ffi::VariableView;
    }

    struct AliasMenuItem {
        plugin: usize,
        id: u16,
        text: QString,
    }

    struct PluginPack {
        id: String,
        name: String,
        path: QString,
        #[cxx_name = "scriptData"]
        script_data: *const u8,
        #[cxx_name = "scriptSize"]
        script_size: usize,
    }

    struct StyledSpan {
        text: QString,
        format: QTextCharFormat,
    }

    enum CommandSource {
        User,
        Execute,
        Hotkey,
        Link,
    }

    #[repr(i32)]
    enum ReplaceSenderResult {
        GroupChanged = -1,
        Unchanged = -2,
        Conflict = -3,
        BadRegularExpression = -4,
        NotFound = -5,
    }

    enum ExportKind {
        Trigger,
        Alias,
        Timer,
        Macro,
        Variable,
        Keypad,
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        #[cxx_name = "SmushClientBase"]
        type SmushClient = super::SmushClientRust;

        fn try_load_world(self: Pin<&mut SmushClient>, path: &QString) -> Result<()>;
        fn try_save_world(self: &SmushClient, path: &QString) -> Result<()>;
        fn try_import_world(self: Pin<&mut SmushClient>, path: &QString) -> Result<RegexParse>;
        fn try_open_log(self: &SmushClient) -> Result<()>;
        fn try_close_log(self: &SmushClient) -> Result<()>;
        fn open_log(self: &SmushClient, path: StringView, append: bool) -> ApiCode;
        fn close_log(self: &SmushClient) -> ApiCode;
        fn flush_log(self: &SmushClient) -> ApiCode;
        fn is_log_open(self: &SmushClient) -> bool;
        fn log_note(self: &SmushClient, note: StringView) -> ApiCode;
        fn log_input(self: &SmushClient, input: &QString) -> ApiCode;
        fn write_to_log(self: &SmushClient, bytes: StringView) -> ApiCode;
        fn load_plugins(self: Pin<&mut SmushClient>) -> QStringList;
        fn world_plugin_index(self: &SmushClient) -> usize;
        fn try_load_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn try_save_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn set_world(self: Pin<&mut SmushClient>, world: &World) -> bool;
        fn connect_to_host(self: &SmushClient, socket: Pin<&mut QAbstractSocket>);
        fn handle_connect(self: &SmushClient, socket: Pin<&mut QAbstractSocket>) -> QString;
        fn handle_disconnect(self: Pin<&mut SmushClient>);
        fn simulate(self: &SmushClient, line: StringView, doc: Pin<&mut Document>);
        fn get_term_color(self: &SmushClient, i: u8) -> QColor;
        fn set_term_color(self: &SmushClient, i: u8, color: &QColor);
        fn get_mapped_color(self: &SmushClient, color: &QColor) -> QColor;
        fn set_mapped_color(self: &SmushClient, color: &QColor, mapped: &QColor);
        fn color_map(self: &SmushClient) -> QList_QPair_QColor_QColor;
        fn world_option(self: &SmushClient, index: usize, option: StringView) -> i64;
        fn world_alpha_option(self: &SmushClient, index: usize, option: StringView)
        -> VariableView;
        fn world_variant_option(self: &SmushClient, index: usize, option: StringView) -> QVariant;
        fn set_world_option(
            self: &SmushClient,
            index: usize,
            option: StringView,
            value: i64,
        ) -> ApiCode;
        fn set_world_alpha_option(
            self: &SmushClient,
            index: usize,
            option: StringView,
            value: StringView,
        ) -> ApiCode;
        fn play_buffer(
            self: &SmushClient,
            i: usize,
            buf: BytesView,
            volume: f32,
            looping: bool,
        ) -> ApiCode;
        fn play_file(
            self: &SmushClient,
            i: usize,
            path: StringView,
            volume: f32,
            looping: bool,
        ) -> ApiCode;
        fn play_file_raw(self: &SmushClient, path: StringView) -> ApiCode;
        fn stop_sound(self: &SmushClient, i: usize) -> ApiCode;
        fn ansi_note(self: &SmushClient, text: StringView) -> Vec<StyledSpan>;
        fn alias(
            self: &SmushClient,
            command: &QString,
            source: CommandSource,
            doc: Pin<&mut Document>,
        ) -> AliasOutcomes;
        fn invoke_alias(self: &SmushClient, index: usize, id: u16, doc: Pin<&mut Document>)
        -> bool;
        fn alias_menu(self: &SmushClient) -> Vec<AliasMenuItem>;
        fn get_info(self: &SmushClient, info_type: i64) -> QVariant;
        fn plugin_info(self: &SmushClient, index: usize, info_type: i64) -> QVariant;
        fn plugins_len(self: &SmushClient) -> usize;
        fn try_add_plugin(self: Pin<&mut SmushClient>, path: &QString) -> Result<usize>;
        fn remove_plugin(self: Pin<&mut SmushClient>, index: usize) -> bool;
        fn plugin_enabled(self: &SmushClient, index: usize) -> bool;
        fn plugin_id(self: &SmushClient, index: usize) -> QString;
        fn plugin_model_text(self: &SmushClient, index: usize, column: i32) -> QString;
        fn plugin(self: &SmushClient, index: usize) -> PluginPack;
        fn reset_world_plugin(self: &SmushClient);
        fn reset_plugins(self: &SmushClient) -> Vec<PluginPack>;
        fn try_reinstall_plugin(self: Pin<&mut SmushClient>, index: usize) -> Result<usize>;
        fn read(
            self: Pin<&mut SmushClient>,
            device: Pin<&mut QAbstractSocket>,
            doc: Pin<&mut Document>,
        ) -> i64;
        fn flush(self: Pin<&mut SmushClient>, doc: Pin<&mut Document>);
        fn handle_alert(self: &SmushClient) -> ApiCode;
        fn has_output(self: &SmushClient) -> bool;
        fn sender_info(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
            info_type: i64,
        ) -> QVariant;
        fn add_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn add_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn add_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn remove_sender(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            name: StringView,
        ) -> ApiCode;
        fn remove_sender_group(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            name: StringView,
        ) -> usize;
        fn remove_sender_groups(self: &SmushClient, index: usize, name: StringView) -> usize;
        fn remove_temporary_senders(self: &SmushClient, kind: SenderKind) -> usize;
        fn add_world_alias(self: &SmushClient, alias: &Alias) -> ApiCode;
        fn add_world_timer(self: &SmushClient, timer: &Timer, timekeeper: &Timekeeper) -> ApiCode;
        fn add_world_trigger(self: &SmushClient, trigger: &Trigger) -> ApiCode;
        fn replace_world_alias(self: &SmushClient, index: usize, alias: &Alias) -> i32;
        fn replace_world_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> i32;
        fn replace_world_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> i32;
        pub fn try_export_xml(
            self: &SmushClient,
            kind: ExportKind,
            index: usize,
            name: StringView,
        ) -> Result<QString>;
        fn try_export_world_senders(self: &SmushClient, kind: SenderKind) -> Result<QString>;
        fn try_import_world_aliases(self: &SmushClient, xml: &QString) -> Result<RegexParse>;
        fn try_import_world_timers(
            self: &SmushClient,
            xml: &QString,
            timekeeper: &Timekeeper,
        ) -> Result<RegexParse>;
        fn try_import_world_triggers(self: &SmushClient, xml: &QString) -> Result<RegexParse>;
        fn replace_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn replace_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn replace_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn is_sender(self: &SmushClient, kind: SenderKind, index: usize, label: StringView)
        -> bool;
        fn set_sender_enabled(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
            enable: bool,
        ) -> ApiCode;
        fn set_sender_group_enabled(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            group: StringView,
            enable: bool,
        ) -> usize;
        fn set_sender_groups_enabled(
            self: &SmushClient,
            index: usize,
            group: StringView,
            enable: bool,
        ) -> usize;
        fn set_plugin_enabled(self: &SmushClient, index: usize, enable: bool);
        fn get_sender_option(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
            option: StringView,
        ) -> QVariant;
        fn set_sender_option(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
            option: StringView,
            value: StringView,
        ) -> ApiCode;
        fn get_alias_wildcard(&self, index: usize, label: StringView, name: StringView) -> String;
        fn get_trigger_wildcard(&self, index: usize, label: StringView, name: StringView)
        -> String;
        fn list_variables(self: &SmushClient, index: usize) -> Vec<String>;
        fn get_variable(self: &SmushClient, index: usize, key: StringView) -> VariableView;
        fn get_metavariable(self: &SmushClient, key: StringView) -> VariableView;
        fn has_metavariable(self: &SmushClient, key: StringView) -> bool;
        fn set_variable(
            self: &SmushClient,
            index: usize,
            key: StringView,
            value: BytesView,
        ) -> bool;
        fn unset_variable(self: &SmushClient, index: usize, key: StringView) -> bool;
        fn set_metavariable(self: &SmushClient, key: StringView, value: BytesView) -> bool;
        fn unset_metavariable(self: &SmushClient, key: StringView) -> bool;
        fn get_mxp_entity(self: &SmushClient, name: StringView) -> VariableView;
        fn set_mxp_entity(self: &SmushClient, name: StringView, value: StringView) -> bool;
        fn bytes_received(self: &SmushClient) -> u64;
        fn list_senders(self: &SmushClient, kind: SenderKind, index: usize) -> Vec<String>;
        fn sender_script(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
        ) -> VariableView;
        fn start_timers(self: &SmushClient, index: usize, timekeeper: &Timekeeper);
        fn start_all_timers(self: &SmushClient, timekeeper: &Timekeeper);
        fn finish_timer(self: Pin<&mut SmushClient>, id: usize) -> bool;
        fn poll_timers(self: Pin<&mut SmushClient>);
        fn stop_senders(self: &SmushClient, kind: SenderKind);
        fn command_splitter(self: &SmushClient) -> u8;

        #[qsignal]
        fn timer_sent(self: Pin<&mut SmushClient>, timer: &SendTimer);
    }
}
