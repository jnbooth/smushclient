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

        include!("smushclient_qt/variableview.h");
        type VariableView = crate::ffi::VariableView;

        include!("smushclient_qt/src/ffi/api_code.cxx.h");
        type ApiCode = crate::ffi::ApiCode;
        include!("smushclient_qt/src/ffi/regex.cxx.h");
        type RegexParse = crate::ffi::RegexParse;
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

    enum CommandSource {
        User,
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

    #[auto_cxx_name]
    extern "RustQt" {
        #[qobject]
        #[cxx_name = "SmushClientBase"]
        type SmushClient = super::SmushClientRust;

        fn load_world(self: Pin<&mut SmushClient>, path: &QString) -> Result<()>;
        fn save_world(self: &SmushClient, path: &QString) -> Result<()>;
        fn import_world(self: Pin<&mut SmushClient>, path: &QString) -> Result<RegexParse>;
        fn open_log(self: Pin<&mut SmushClient>) -> Result<()>;
        fn close_log(self: &SmushClient) -> Result<()>;
        fn log_input(self: &SmushClient, input: &QString);
        fn log_note(self: &SmushClient, note: &QString);
        fn load_plugins(self: Pin<&mut SmushClient>) -> QStringList;
        fn world_plugin_index(self: &SmushClient) -> usize;
        fn load_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn save_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn set_world(self: Pin<&mut SmushClient>, world: &World) -> Result<bool>;
        fn connect_to_host(self: &SmushClient, socket: Pin<&mut QAbstractSocket>);
        fn handle_connect(self: &SmushClient, socket: Pin<&mut QAbstractSocket>) -> QString;
        fn handle_disconnect(self: Pin<&mut SmushClient>);
        fn simulate(self: &SmushClient, line: &[u8], doc: Pin<&mut Document>);
        fn world_option(self: &SmushClient, index: usize, option: &[u8]) -> i64;
        fn world_alpha_option(self: &SmushClient, index: usize, option: &[u8]) -> VariableView;
        fn world_variant_option(self: &SmushClient, index: usize, option: &[u8]) -> QVariant;
        fn set_world_option(
            self: Pin<&mut SmushClient>,
            index: usize,
            option: &[u8],
            value: i64,
        ) -> ApiCode;
        fn set_world_alpha_option(
            self: Pin<&mut SmushClient>,
            index: usize,
            option: &[u8],
            value: &[u8],
        ) -> ApiCode;
        fn play_buffer(
            self: &SmushClient,
            i: usize,
            buf: &[u8],
            volume: f32,
            looping: bool,
        ) -> ApiCode;
        fn play_file(
            self: &SmushClient,
            i: usize,
            path: &[u8],
            volume: f32,
            looping: bool,
        ) -> ApiCode;
        fn play_file_raw(self: &SmushClient, path: &[u8]) -> ApiCode;
        fn stop_sound(self: &SmushClient, i: usize) -> ApiCode;
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
        fn plugin_info(self: &SmushClient, index: usize, info_type: u8) -> QVariant;
        fn plugins_len(self: &SmushClient) -> usize;
        fn add_plugin(self: Pin<&mut SmushClient>, path: &QString) -> Result<usize>;
        fn remove_plugin(self: Pin<&mut SmushClient>, index: usize) -> bool;
        fn plugin_enabled(self: &SmushClient, index: usize) -> bool;
        fn plugin_id(self: &SmushClient, index: usize) -> QString;
        fn plugin_model_text(self: &SmushClient, index: usize, column: i32) -> QString;
        fn plugin(self: &SmushClient, index: usize) -> PluginPack;
        fn reset_world_plugin(self: &SmushClient);
        fn reset_plugins(self: &SmushClient) -> Vec<PluginPack>;
        fn reinstall_plugin(self: Pin<&mut SmushClient>, index: usize) -> Result<usize>;
        fn read(
            self: Pin<&mut SmushClient>,
            device: Pin<&mut QAbstractSocket>,
            doc: Pin<&mut Document>,
        ) -> i64;
        fn flush(self: Pin<&mut SmushClient>, doc: Pin<&mut Document>);
        fn handle_alert(self: &SmushClient) -> ApiCode;
        fn has_output(self: &SmushClient) -> bool;
        fn timer_info(self: &SmushClient, index: usize, label: &[u8], info_type: u8) -> QVariant;
        fn add_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn add_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn add_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn remove_alias(self: &SmushClient, index: usize, label: &[u8]) -> ApiCode;
        fn remove_timer(self: &SmushClient, index: usize, label: &[u8]) -> ApiCode;
        fn remove_trigger(self: &SmushClient, index: usize, label: &[u8]) -> ApiCode;
        fn remove_alias_group(self: &SmushClient, index: usize, group: &[u8]) -> usize;
        fn remove_timer_group(self: &SmushClient, index: usize, group: &[u8]) -> usize;
        fn remove_trigger_group(self: &SmushClient, index: usize, group: &[u8]) -> usize;
        fn add_world_alias(self: &SmushClient, alias: &Alias) -> Result<ApiCode>;
        fn add_world_timer(self: &SmushClient, timer: &Timer, timekeeper: &Timekeeper) -> ApiCode;
        fn add_world_trigger(self: &SmushClient, trigger: &Trigger) -> Result<ApiCode>;
        fn replace_world_alias(self: &SmushClient, index: usize, alias: &Alias) -> i32;
        fn replace_world_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> i32;
        fn replace_world_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> i32;
        fn export_world_aliases(self: &SmushClient) -> Result<QString>;
        fn export_world_timers(self: &SmushClient) -> Result<QString>;
        fn export_world_triggers(self: &SmushClient) -> Result<QString>;
        fn import_world_aliases(self: &SmushClient, xml: &QString) -> Result<RegexParse>;
        fn import_world_timers(
            self: &SmushClient,
            xml: &QString,
            timekeeper: &Timekeeper,
        ) -> Result<RegexParse>;
        fn import_world_triggers(self: &SmushClient, xml: &QString) -> Result<RegexParse>;
        fn replace_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn replace_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn replace_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn is_alias(self: &SmushClient, index: usize, label: &[u8]) -> bool;
        fn is_timer(self: &SmushClient, index: usize, label: &[u8]) -> bool;
        fn is_trigger(self: &SmushClient, index: usize, label: &[u8]) -> bool;
        fn set_alias_enabled(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            enable: bool,
        ) -> ApiCode;
        fn set_aliases_enabled(
            self: &SmushClient,
            index: usize,
            group: &[u8],
            enable: bool,
        ) -> bool;
        fn set_plugin_enabled(self: &SmushClient, index: usize, enable: bool) -> bool;
        fn set_timer_enabled(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            enable: bool,
        ) -> ApiCode;
        fn set_timers_enabled(self: &SmushClient, index: usize, group: &[u8], enable: bool)
        -> bool;
        fn set_trigger_enabled(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            enable: bool,
        ) -> ApiCode;
        fn set_triggers_enabled(
            self: &SmushClient,
            index: usize,
            group: &[u8],
            enable: bool,
        ) -> bool;
        fn get_alias_option(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            option: &[u8],
        ) -> QVariant;
        fn get_timer_option(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            option: &[u8],
        ) -> QVariant;
        fn get_trigger_option(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            option: &[u8],
        ) -> QVariant;
        fn set_alias_option(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            option: &[u8],
            value: &[u8],
        ) -> ApiCode;
        fn set_timer_option(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            option: &[u8],
            value: &[u8],
        ) -> ApiCode;
        fn set_trigger_option(
            self: &SmushClient,
            index: usize,
            label: &[u8],
            option: &[u8],
            value: &[u8],
        ) -> ApiCode;
        fn get_variable(self: &SmushClient, index: usize, key: &[u8]) -> VariableView;
        fn get_metavariable(self: &SmushClient, key: &[u8]) -> VariableView;
        fn has_metavariable(self: &SmushClient, key: &[u8]) -> bool;
        fn set_variable(self: &SmushClient, index: usize, key: &[u8], value: &[u8]) -> bool;
        fn unset_variable(self: &SmushClient, index: usize, key: &[u8]) -> bool;
        fn set_metavariable(self: &SmushClient, key: &[u8], value: &[u8]) -> bool;
        fn unset_metavariable(self: &SmushClient, key: &[u8]) -> bool;
        fn start_timers(self: &SmushClient, index: usize, timekeeper: &Timekeeper);
        fn start_all_timers(self: &SmushClient, timekeeper: &Timekeeper);
        fn finish_timer(self: &SmushClient, id: usize, timekeeper: &Timekeeper) -> bool;
        fn poll_timers(self: &SmushClient, timekeeper: &Timekeeper);
        fn stop_senders(self: &SmushClient);
        fn stop_aliases(self: &SmushClient);
        fn stop_timers(self: &SmushClient);
        fn stop_triggers(self: &SmushClient);
        fn command_splitter(self: &SmushClient) -> u8;
    }
}
