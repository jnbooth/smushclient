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
        #[cxx_name = "AbstractDocument"]
        type Document = crate::ffi::Document;
        #[cxx_name = "AbstractTimekeeper"]
        type Timekeeper = crate::ffi::Timekeeper;
        type Timer = crate::ffi::Timer;
        type Trigger = crate::ffi::Trigger;
        type World = crate::ffi::World;

        include!("smushclient_qt/src/ffi/api_code.cxx.h");
        type ApiCode = crate::ffi::ApiCode;
        include!("smushclient_qt/src/ffi/regex.cxx.h");
        type ParseResult = crate::ffi::ParseResult;
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

    struct VariableEntry {
        key: VariableView,
        value: VariableView,
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

    #[repr(i64)]
    enum AudioSinkStatus {
        OutOfRange = -1,
        Done = 0,
        Playing = 1,
        Looping = 2,
    }

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        #[cxx_name = "SmushClientBase"]
        type SmushClient = super::SmushClientRust;

        // color
        fn ansi_note(self: &SmushClient, text: StringView) -> Vec<StyledSpan>;
        fn color_map(self: &SmushClient) -> QList_QPair_QColor_QColor;
        fn get_mapped_color(self: &SmushClient, color: &QColor) -> QColor;
        fn get_term_color(self: &SmushClient, i: u8) -> QColor;
        fn set_mapped_color(self: &SmushClient, color: &QColor, mapped: &QColor);
        fn set_term_color(self: &SmushClient, i: u8, color: &QColor);

        // log
        fn close_log(self: &SmushClient) -> ApiCode;
        fn flush_log(self: &SmushClient) -> ApiCode;
        fn is_log_open(self: &SmushClient) -> bool;
        fn log_input(self: &SmushClient, input: &QString) -> ApiCode;
        fn log_note(self: &SmushClient, note: StringView) -> ApiCode;
        fn open_log(self: &SmushClient, path: StringView, append: bool) -> ApiCode;
        fn try_close_log(self: &SmushClient) -> Result<()>;
        fn try_open_log(self: &SmushClient) -> Result<()>;
        fn write_to_log(self: &SmushClient, bytes: BytesView) -> ApiCode;

        // network
        fn bytes_received(self: &SmushClient) -> u64;
        fn connect_to_host(self: &SmushClient, socket: Pin<&mut QAbstractSocket>);
        fn flush(self: &SmushClient, doc: Pin<&mut Document>);
        fn handle_connect(self: &SmushClient, socket: Pin<&mut QAbstractSocket>) -> QString;
        fn handle_disconnect(self: &SmushClient);
        fn has_output(self: &SmushClient) -> bool;
        fn read(
            self: &SmushClient,
            device: Pin<&mut QAbstractSocket>,
            doc: Pin<&mut Document>,
        ) -> i64;
        fn reset_mxp(self: &SmushClient);

        // option
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
        fn set_world_alpha_option(
            self: &SmushClient,
            index: usize,
            option: StringView,
            value: StringView,
        ) -> ApiCode;
        fn set_world_option(
            self: &SmushClient,
            index: usize,
            option: StringView,
            value: i64,
        ) -> ApiCode;
        fn world_alpha_option(self: &SmushClient, index: usize, option: StringView)
        -> VariableView;
        fn world_option(self: &SmushClient, index: usize, option: StringView) -> i64;
        fn world_variant_option(self: &SmushClient, index: usize, option: StringView) -> QVariant;

        // plugin
        fn load_plugins(self: Pin<&mut SmushClient>) -> QStringList;
        fn plugin(self: &SmushClient, index: usize) -> PluginPack;
        fn plugin_enabled(self: &SmushClient, index: usize) -> bool;
        fn plugin_id(self: &SmushClient, index: usize) -> QString;
        fn plugin_info(self: &SmushClient, index: usize, info_type: i64) -> QVariant;
        fn plugins_len(self: &SmushClient) -> usize;
        fn plugin_model_text(self: &SmushClient, index: usize, column: i32) -> QString;
        fn remove_plugin(self: Pin<&mut SmushClient>, index: usize) -> bool;
        fn reset_plugins(self: &SmushClient) -> Vec<PluginPack>;
        fn reset_world_plugin(self: &SmushClient);
        fn set_plugin_enabled(self: &SmushClient, index: usize, enable: bool);
        fn try_add_plugin(self: Pin<&mut SmushClient>, path: &QString) -> Result<usize>;
        fn try_reinstall_plugin(self: Pin<&mut SmushClient>, index: usize) -> Result<usize>;
        fn world_plugin_index(self: &SmushClient) -> usize;

        // sender
        fn add_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn add_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn add_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn add_world_alias(self: &SmushClient, alias: &Alias) -> ApiCode;
        fn add_world_timer(self: &SmushClient, timer: &Timer, timekeeper: &Timekeeper) -> ApiCode;
        fn add_world_trigger(self: &SmushClient, trigger: &Trigger) -> ApiCode;
        fn alias(
            self: &SmushClient,
            command: &QString,
            source: CommandSource,
            doc: Pin<&mut Document>,
        ) -> AliasOutcomes;
        fn alias_menu(self: &SmushClient) -> Vec<AliasMenuItem>;
        fn finish_timer(self: Pin<&mut SmushClient>, id: usize) -> bool;
        fn get_alias_wildcard(&self, index: usize, label: StringView, name: StringView) -> String;
        fn get_trigger_wildcard(&self, index: usize, label: StringView, name: StringView)
        -> String;
        fn invoke_alias(self: &SmushClient, index: usize, id: u16, doc: Pin<&mut Document>)
        -> bool;
        fn is_sender(self: &SmushClient, kind: SenderKind, index: usize, label: StringView)
        -> bool;
        fn list_senders(self: &SmushClient, kind: SenderKind, index: usize) -> Vec<String>;
        fn poll_timers(self: Pin<&mut SmushClient>);
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
        fn replace_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn replace_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn replace_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn replace_world_alias(self: &SmushClient, index: usize, alias: &Alias) -> i32;
        fn replace_world_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> i32;
        fn replace_world_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> i32;
        fn sender_script(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
        ) -> VariableView;
        fn sender_info(
            self: &SmushClient,
            kind: SenderKind,
            index: usize,
            label: StringView,
            info_type: i64,
        ) -> QVariant;
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
        fn simulate(self: &SmushClient, line: StringView, doc: Pin<&mut Document>);
        fn start_all_timers(self: &SmushClient, timekeeper: &Timekeeper);
        fn start_timers(self: &SmushClient, index: usize, timekeeper: &Timekeeper);
        fn stop_senders(self: &SmushClient, kind: SenderKind);

        // sound
        fn handle_alert(self: &SmushClient) -> ApiCode;
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
        fn sound_status(self: &SmushClient, i: usize) -> AudioSinkStatus;
        fn stop_sound(self: &SmushClient, i: usize) -> ApiCode;

        // variable
        fn get_metavariable(self: &SmushClient, key: StringView) -> VariableView;
        fn get_mxp_entity(self: &SmushClient, name: StringView) -> VariableView;
        fn get_variable(self: &SmushClient, index: usize, key: StringView) -> VariableView;
        fn has_metavariable(self: &SmushClient, key: StringView) -> bool;
        fn variable_entries(self: &SmushClient, index: usize) -> Vec<VariableEntry>;
        fn set_metavariable(self: &SmushClient, key: StringView, value: BytesView) -> bool;
        fn set_mxp_entity(self: &SmushClient, name: StringView, value: StringView) -> bool;
        fn set_variable(
            self: &SmushClient,
            index: usize,
            key: StringView,
            value: BytesView,
        ) -> bool;
        fn try_load_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn try_save_state(self: &SmushClient, index: usize, path: &QString) -> Result<()>;
        fn try_save_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn unset_metavariable(self: &SmushClient, key: StringView) -> bool;
        fn unset_variable(self: &SmushClient, index: usize, key: StringView) -> bool;

        // world
        fn command_splitter(self: &SmushClient) -> u16;
        fn evaluate_speedwalk(self: &SmushClient, speedwalk: StringView) -> String;
        fn get_info(self: &SmushClient, info_type: i64) -> QVariant;
        fn set_world(self: Pin<&mut SmushClient>, world: &World) -> bool;
        fn speed_walk_prefix(self: &SmushClient) -> u16;
        fn try_evaluate_speedwalk(self: &SmushClient, speedwalk: &QString) -> Result<QString>;
        fn try_load_world(self: Pin<&mut SmushClient>, path: &QString) -> Result<()>;
        fn try_save_world(self: &SmushClient, path: &QString) -> Result<()>;

        // xml
        fn import_world(self: Pin<&mut SmushClient>, path: &QString) -> ParseResult;
        fn import_world_aliases(self: &SmushClient, xml: &QString) -> ParseResult;
        fn import_world_timers(
            self: &SmushClient,
            xml: &QString,
            timekeeper: &Timekeeper,
        ) -> ParseResult;
        fn import_world_triggers(self: &SmushClient, xml: &QString) -> ParseResult;
        fn import_xml(self: &SmushClient, xml: StringView) -> ParseResult;
        pub fn try_export_xml(
            self: &SmushClient,
            kind: ExportKind,
            index: usize,
            name: StringView,
        ) -> Result<QString>;
        fn try_export_world_senders(self: &SmushClient, kind: SenderKind) -> Result<QString>;

        // Qt
        #[qsignal]
        fn timer_sent(self: Pin<&mut SmushClient>, timer: &SendTimer);
    }
}
