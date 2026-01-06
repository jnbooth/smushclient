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
        include!("forward.h");
        type Alias = crate::ffi::Alias;
        type AliasOutcomes = crate::ffi::AliasOutcomes;
        type Document = crate::ffi::Document;
        type Timekeeper = crate::ffi::Timekeeper;
        type Timer = crate::ffi::Timer;
        type Trigger = crate::ffi::Trigger;
        type World = crate::ffi::World;

        include!("variableview.h");
        type VariableView = crate::ffi::VariableView;

        include!("smushclient_qt/src/ffi/api_code.cxx.h");
        type ApiCode = crate::ffi::ApiCode;
    }

    struct PluginPack {
        id: QString,
        name: QString,
        path: QString,
        #[cxx_name = "scriptData"]
        script_data: *const u8,
        #[cxx_name = "scriptSize"]
        script_size: usize,
    }

    enum AliasBool {
        // Sender
        Enabled,
        OneShot,
        Temporary,
        OmitFromOutput,
        OmitFromLog,
        // Reaction
        IgnoreCase,
        KeepEvaluating,
        IsRegex,
        ExpandVariables,
        Repeats,
        // Alias
        EchoAlias,
        Menu,
        OmitFromCommandHistory,
    }

    enum TimerBool {
        // Sender
        Enabled,
        OneShot,
        Temporary,
        OmitFromOutput,
        OmitFromLog,
        // Timer
        ActiveClosed,
    }

    enum TriggerBool {
        // Sender
        Enabled,
        OneShot,
        Temporary,
        OmitFromOutput,
        OmitFromLog,
        // Reaction
        IgnoreCase,
        KeepEvaluating,
        IsRegex,
        ExpandVariables,
        Repeats,
        // Trigger
        ChangeForeground,
        ChangeBackground,
        MakeBold,
        MakeItalic,
        MakeUnderline,
        SoundIfInactive,
        LowercaseWildcard,
        MultiLine,
    }

    enum CommandSource {
        Hotkey,
        Link,
        User,
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

        fn load_world(
            self: Pin<&mut SmushClient>,
            path: &QString,
            world: Pin<&mut World>,
        ) -> Result<()>;
        fn save_world(self: &SmushClient, path: &QString) -> Result<()>;
        fn open_log(self: Pin<&mut SmushClient>) -> Result<()>;
        fn close_log(self: &SmushClient) -> Result<()>;
        fn load_plugins(self: Pin<&mut SmushClient>) -> QStringList;
        fn world_plugin_index(self: &SmushClient) -> usize;
        fn load_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn save_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn populate_world(self: &SmushClient, world: Pin<&mut World>);
        fn set_world(self: Pin<&mut SmushClient>, world: &World) -> Result<bool>;
        fn handle_connect(self: &SmushClient, socket: Pin<&mut QAbstractSocket>) -> QString;
        fn handle_disconnect(self: Pin<&mut SmushClient>);
        fn play_file(
            self: &SmushClient,
            i: usize,
            path: &QString,
            volume: f32,
            looping: bool,
        ) -> ApiCode;
        fn play_buffer(
            self: &SmushClient,
            i: usize,
            buf: &[u8],
            volume: f32,
            looping: bool,
        ) -> ApiCode;
        fn stop_sound(self: &SmushClient, i: usize) -> ApiCode;
        fn alias(
            self: &SmushClient,
            command: &QString,
            source: CommandSource,
            doc: Pin<&mut Document>,
        ) -> AliasOutcomes;
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
        fn has_output(self: &SmushClient) -> bool;
        fn timer_info(self: &SmushClient, index: usize, label: &QString, info_type: u8)
        -> QVariant;
        fn add_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn add_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn add_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn remove_alias(self: &SmushClient, index: usize, label: &QString) -> ApiCode;
        fn remove_timer(self: &SmushClient, index: usize, label: &QString) -> ApiCode;
        fn remove_trigger(self: &SmushClient, index: usize, label: &QString) -> ApiCode;
        fn remove_aliases(self: &SmushClient, index: usize, group: &QString) -> usize;
        fn remove_timers(self: &SmushClient, index: usize, group: &QString) -> usize;
        fn remove_triggers(self: &SmushClient, index: usize, group: &QString) -> usize;
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
        fn import_world_aliases(self: &SmushClient, xml: &QString) -> Result<()>;
        fn import_world_timers(
            self: &SmushClient,
            xml: &QString,
            timekeeper: &Timekeeper,
        ) -> Result<()>;
        fn import_world_triggers(self: &SmushClient, xml: &QString) -> Result<()>;
        fn replace_alias(self: &SmushClient, index: usize, alias: &Alias) -> ApiCode;
        fn replace_timer(
            self: &SmushClient,
            index: usize,
            timer: &Timer,
            timekeeper: &Timekeeper,
        ) -> ApiCode;
        fn replace_trigger(self: &SmushClient, index: usize, trigger: &Trigger) -> ApiCode;
        fn is_alias(self: &SmushClient, index: usize, label: &QString) -> bool;
        fn is_timer(self: &SmushClient, index: usize, label: &QString) -> bool;
        fn is_trigger(self: &SmushClient, index: usize, label: &QString) -> bool;
        fn set_alias_enabled(
            self: &SmushClient,
            index: usize,
            label: &QString,
            enable: bool,
        ) -> ApiCode;
        fn set_aliases_enabled(
            self: &SmushClient,
            index: usize,
            group: &QString,
            enable: bool,
        ) -> bool;
        fn set_plugin_enabled(self: &SmushClient, index: usize, enable: bool) -> bool;
        fn set_timer_enabled(
            self: &SmushClient,
            index: usize,
            label: &QString,
            enable: bool,
        ) -> ApiCode;
        fn set_timers_enabled(
            self: &SmushClient,
            index: usize,
            group: &QString,
            enable: bool,
        ) -> bool;
        fn set_trigger_enabled(
            self: &SmushClient,
            index: usize,
            label: &QString,
            enable: bool,
        ) -> ApiCode;
        fn set_triggers_enabled(
            self: &SmushClient,
            index: usize,
            group: &QString,
            enable: bool,
        ) -> bool;
        fn set_alias_bool(
            self: &SmushClient,
            index: usize,
            label: &QString,
            option: AliasBool,
            value: bool,
        ) -> ApiCode;
        fn set_timer_bool(
            self: &SmushClient,
            index: usize,
            label: &QString,
            option: TimerBool,
            value: bool,
        ) -> ApiCode;
        fn set_trigger_bool(
            self: &SmushClient,
            index: usize,
            label: &QString,
            option: TriggerBool,
            value: bool,
        ) -> ApiCode;
        fn set_trigger_group(
            self: &SmushClient,
            index: usize,
            label: &QString,
            group: &QString,
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
    }
}
