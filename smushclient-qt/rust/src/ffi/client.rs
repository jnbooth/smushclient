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
        include!(<QtNetwork/QAbstractSocket>);
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
    }

    #[repr(i32)]
    enum SenderAccessResult {
        LabelConflict = -4,
        BadParameter = -3,
        NotFound = -2,
        Unchanged = -1,
        Ok = 0,
    }

    enum SoundResult {
        Ok = 0,
        SoundError = 1,
        BadParameter = 2,
        NotFound = 3,
    }

    struct PluginPack {
        id: QString,
        name: QString,
        path: QString,
        scriptData: *const u8,
        scriptSize: usize,
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
        fn close_log(self: Pin<&mut SmushClient>);
        fn load_plugins(self: Pin<&mut SmushClient>) -> QStringList;
        fn world_plugin_index(self: &SmushClient) -> usize;
        fn load_variables(self: Pin<&mut SmushClient>, path: &QString) -> Result<bool>;
        fn save_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn populate_world(self: &SmushClient, world: Pin<&mut World>);
        fn set_world(self: Pin<&mut SmushClient>, world: &World) -> bool;
        fn palette(self: &SmushClient) -> QVector_QColor;
        fn handle_connect(self: &SmushClient, socket: Pin<&mut QAbstractSocket>) -> QString;
        fn handle_disconnect(self: Pin<&mut SmushClient>);
        fn play_file(
            self: &SmushClient,
            i: usize,
            path: &QString,
            volume: f32,
            looping: bool,
        ) -> SoundResult;
        fn play_buffer(
            self: &SmushClient,
            i: usize,
            buf: &[u8],
            volume: f32,
            looping: bool,
        ) -> SoundResult;
        fn stop_sound(self: &SmushClient, i: usize) -> SoundResult;
        fn alias(
            self: Pin<&mut SmushClient>,
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
        fn reset_world_plugin(self: Pin<&mut SmushClient>);
        fn reset_plugins(self: Pin<&mut SmushClient>) -> Vec<PluginPack>;
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
        fn add_alias(self: Pin<&mut SmushClient>, index: usize, alias: &Alias) -> Result<i32>;
        fn add_timer(
            self: Pin<&mut SmushClient>,
            index: usize,
            timer: &Timer,
            timekeper: Pin<&mut Timekeeper>,
        ) -> i32;
        fn add_trigger(self: Pin<&mut SmushClient>, index: usize, trigger: &Trigger)
        -> Result<i32>;
        fn remove_alias(self: Pin<&mut SmushClient>, index: usize, label: &QString) -> i32;
        fn remove_timer(self: Pin<&mut SmushClient>, index: usize, label: &QString) -> i32;
        fn remove_trigger(self: Pin<&mut SmushClient>, index: usize, label: &QString) -> i32;
        fn remove_aliases(self: Pin<&mut SmushClient>, index: usize, group: &QString) -> usize;
        fn remove_timers(self: Pin<&mut SmushClient>, index: usize, group: &QString) -> usize;
        fn remove_triggers(self: Pin<&mut SmushClient>, index: usize, group: &QString) -> usize;
        fn add_world_alias(self: Pin<&mut SmushClient>, alias: &Alias) -> Result<i32>;
        fn add_world_timer(
            self: Pin<&mut SmushClient>,
            timer: &Timer,
            timekeeper: Pin<&mut Timekeeper>,
        ) -> i32;
        fn add_world_trigger(self: Pin<&mut SmushClient>, trigger: &Trigger) -> Result<i32>;
        fn replace_world_alias(
            self: Pin<&mut SmushClient>,
            index: usize,
            alias: &Alias,
        ) -> Result<i32>;
        fn replace_world_timer(
            self: Pin<&mut SmushClient>,
            index: usize,
            timer: &Timer,
            timekeeper: Pin<&mut Timekeeper>,
        ) -> i32;
        fn replace_world_trigger(
            self: Pin<&mut SmushClient>,
            index: usize,
            trigger: &Trigger,
        ) -> Result<i32>;
        fn remove_world_alias(self: Pin<&mut SmushClient>, i: usize) -> bool;
        fn remove_world_timer(self: Pin<&mut SmushClient>, i: usize) -> bool;
        fn remove_world_trigger(self: Pin<&mut SmushClient>, i: usize) -> bool;
        fn export_world_aliases(self: &SmushClient) -> Result<QString>;
        fn export_world_timers(self: &SmushClient) -> Result<QString>;
        fn export_world_triggers(self: &SmushClient) -> Result<QString>;
        fn import_world_aliases(self: Pin<&mut SmushClient>, xml: &QString) -> Result<()>;
        fn import_world_timers(
            self: Pin<&mut SmushClient>,
            xml: &QString,
            timekeeper: Pin<&mut Timekeeper>,
        ) -> Result<()>;
        fn import_world_triggers(self: Pin<&mut SmushClient>, xml: &QString) -> Result<()>;
        fn replace_alias(self: Pin<&mut SmushClient>, index: usize, alias: &Alias) -> Result<()>;
        fn replace_timer(
            self: Pin<&mut SmushClient>,
            index: usize,
            timer: &Timer,
            timekeeper: Pin<&mut Timekeeper>,
        );
        fn replace_trigger(
            self: Pin<&mut SmushClient>,
            index: usize,
            trigger: &Trigger,
        ) -> Result<()>;
        fn is_alias(self: &SmushClient, index: usize, label: &QString) -> bool;
        fn is_timer(self: &SmushClient, index: usize, label: &QString) -> bool;
        fn is_trigger(self: &SmushClient, index: usize, label: &QString) -> bool;
        fn set_alias_enabled(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            enable: bool,
        ) -> i32;
        fn set_aliases_enabled(
            self: Pin<&mut SmushClient>,
            index: usize,
            group: &QString,
            enable: bool,
        ) -> bool;
        fn set_plugin_enabled(self: Pin<&mut SmushClient>, index: usize, enable: bool) -> bool;
        fn set_timer_enabled(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            enable: bool,
        ) -> i32;
        fn set_timers_enabled(
            self: Pin<&mut SmushClient>,
            index: usize,
            group: &QString,
            enable: bool,
        ) -> bool;
        fn set_trigger_enabled(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            enable: bool,
        ) -> i32;
        fn set_triggers_enabled(
            self: Pin<&mut SmushClient>,
            index: usize,
            group: &QString,
            enable: bool,
        ) -> bool;
        fn set_alias_bool(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            option: AliasBool,
            value: bool,
        ) -> i32;
        fn set_timer_bool(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            option: TimerBool,
            value: bool,
        ) -> i32;
        fn set_trigger_bool(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            option: TriggerBool,
            value: bool,
        ) -> i32;
        fn set_trigger_group(
            self: Pin<&mut SmushClient>,
            index: usize,
            label: &QString,
            group: &QString,
        ) -> i32;
        /// # Safety
        ///
        /// `value_size` must be valid or null.
        unsafe fn get_variable(
            self: &SmushClient,
            index: usize,
            key: &[c_char],
            value_size: *mut usize,
        ) -> *const c_char;
        /// # Safety
        ///
        /// `value_size` must be valid or null.
        unsafe fn get_metavariable(
            self: &SmushClient,
            key: &[c_char],
            value_size: *mut usize,
        ) -> *const c_char;
        fn has_metavariable(self: &SmushClient, key: &[c_char]) -> bool;
        fn set_variable(
            self: Pin<&mut SmushClient>,
            index: usize,
            key: &[c_char],
            value: &[c_char],
        ) -> bool;
        fn unset_variable(self: Pin<&mut SmushClient>, index: usize, key: &[c_char]) -> bool;
        fn set_metavariable(self: Pin<&mut SmushClient>, key: &[c_char], value: &[c_char]) -> bool;
        fn unset_metavariable(self: Pin<&mut SmushClient>, key: &[c_char]) -> bool;
        fn start_timers(
            self: Pin<&mut SmushClient>,
            index: usize,
            timekeeper: Pin<&mut Timekeeper>,
        );
        fn finish_timer(
            self: Pin<&mut SmushClient>,
            id: usize,
            timekeeper: Pin<&mut Timekeeper>,
        ) -> bool;
        fn poll_timers(self: Pin<&mut SmushClient>, timekeeper: Pin<&mut Timekeeper>);
        fn stop_senders(self: Pin<&mut SmushClient>);
        fn stop_aliases(self: Pin<&mut SmushClient>);
        fn stop_timers(self: Pin<&mut SmushClient>);
        fn stop_triggers(self: Pin<&mut SmushClient>);
    }
}
