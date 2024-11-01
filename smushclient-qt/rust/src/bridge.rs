#![allow(clippy::float_cmp)]
#![allow(clippy::missing_safety_doc)]
#![allow(clippy::needless_lifetimes)]
#![allow(clippy::needless_pass_by_value)]
#![allow(clippy::unnecessary_box_returns)]
#![allow(non_snake_case)]

use std::mem;

use crate::client::SmushClientRust;
use crate::sender::{AliasRust, ReactionRust, SenderRust, TimerRust, TriggerRust};
use crate::sender::{OutputSpan, TextSpan};
use crate::world::WorldRust;
use cxx::{type_id, ExternType};
use cxx_qt_lib::QByteArray;
use mud_transformer::escape::telnet::naws;

#[repr(transparent)]
pub struct AliasOutcomes(pub u8);
const _: [(); mem::size_of::<AliasOutcomes>()] = [(); mem::size_of::<ffi::AliasOutcome>()];

unsafe impl ExternType for AliasOutcomes {
    type Id = type_id!("AliasOutcomes");
    type Kind = cxx::kind::Trivial;
}

#[repr(transparent)]
pub struct TextStyles(pub u16);
const _: [(); mem::size_of::<TextStyles>()] = [(); mem::size_of::<ffi::TextStyle>()];

unsafe impl ExternType for TextStyles {
    type Id = type_id!("TextStyles");
    type Kind = cxx::kind::Trivial;
}

fn encode_naws(width: u16, height: u16) -> QByteArray {
    QByteArray::from(naws(width, height).as_slice())
}

#[cxx_qt::bridge]
pub mod ffi {
    enum AliasOutcome {
        Remember = 1,
        Send = 2,
        Display = 4,
    }

    #[repr(i32)]
    enum SenderAccessResult {
        LabelConflict = -4,
        BadParameter = -3,
        Locked = -2,
        NotFound = -1,
        Ok = 0,
    }

    enum TextStyle {
        Blink = 1,
        Bold = 2,
        Highlight = 4,
        Italic = 8,
        NonProportional = 16,
        Small = 32,
        Strikeout = 64,
        Underline = 128,
        Inverse = 256,
    }

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

    unsafe extern "C++" {
        include!("cxx-qt-lib/qtime.h");
        type QTime = cxx_qt_lib::QTime;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qvariant.h");
        type QVariant = cxx_qt_lib::QVariant;
    }

    unsafe extern "C++" {
        include!("cxx-qt-lib/qvector.h");
        type QVector_QColor = cxx_qt_lib::QVector<QColor>;
    }

    extern "C++Qt" {
        include!(<QtNetwork/QTcpSocket>);
        type QTcpSocket;

        unsafe fn flush(self: Pin<&mut QTcpSocket>) -> bool;
        unsafe fn read(self: Pin<&mut QTcpSocket>, data: *mut c_char, max_size: i64) -> i64;
        unsafe fn write(self: Pin<&mut QTcpSocket>, data: *const c_char, max_size: i64) -> i64;
    }

    extern "Rust" {
        #[cxx_name = "encodeNaws"]
        fn encode_naws(width: u16, height: u16) -> QByteArray;
    }

    enum TelnetSource {
        Client,
        Server,
    }

    enum TelnetVerb {
        Do,
        Dont,
        Will,
        Wont,
    }

    struct PluginPack {
        id: QString,
        name: QString,
        path: QString,
        scriptData: *const u8,
        scriptSize: usize,
    }

    enum SendTo {
        Internet,
        World,
        Input,
    }

    struct Link {
        action: QString,
        hint: QString,
        prompts: QString,
        sendto: SendTo,
    }

    #[qenum(Sender)]
    enum SendTarget {
        World,
        Command,
        Output,
        Status,
        NotepadNew,
        NotepadAppend,
        Log,
        NotepadReplace,
        WorldDelay,
        Variable,
        Execute,
        Speedwalk,
        Script,
        WorldImmediate,
        ScriptAfterOmit,
    }

    struct SendTimer {
        #[rust_name = "active_closed"]
        activeClosed: bool,
        label: String,
        plugin: usize,
        script: String,
        target: SendTarget,
        text: QString,
    }

    struct SendRequest {
        plugin: usize,
        send_to: SendTarget,
        text: QString,
        pad: QString,
    }

    struct NamedWildcard<'a> {
        name: &'a str,
        value: &'a str,
    }

    struct SendScriptRequest<'a> {
        plugin: usize,
        script: &'a str,
        label: &'a str,
        line: &'a str,
        wildcards: Vec<&'a str>,
        #[rust_name = "named_wildcards"]
        namedWildcards: Vec<NamedWildcard<'a>>,
        output: &'a [OutputSpan],
    }

    extern "Rust" {
        type TextSpan;
        fn foreground(&self) -> i32;
        fn background(&self) -> i32;
        fn text(&self) -> &str;
        fn style(&self) -> u8;
    }

    extern "Rust" {
        type OutputSpan;
        fn text_span(&self) -> *const TextSpan;
    }

    extern "C++Qt" {
        include!("document.h");
        type AliasOutcomes = super::AliasOutcomes;
        type TextStyles = super::TextStyles;

        type Document;

        #[rust_name = "append_html"]
        unsafe fn appendHtml(self: &Document, text: &QString);

        #[rust_name = "append_line"]
        unsafe fn appendLine(self: Pin<&mut Document>);

        #[rust_name = "append_plaintext"]
        unsafe fn appendText(self: &Document, text: &QString, palette: i32);

        #[rust_name = "append_text"]
        unsafe fn appendText(
            self: &Document,
            text: &QString,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
        );

        #[rust_name = "append_link"]
        unsafe fn appendText(
            self: &Document,
            text: &QString,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
            link: &Link,
        );

        #[rust_name = "apply_styles"]
        unsafe fn applyStyles(
            self: &Document,
            start: i32,
            end: i32,
            style: TextStyles,
            foreground: &QColor,
            background: &QColor,
        );

        unsafe fn beep(self: &Document);

        unsafe fn begin(self: &Document);

        #[rust_name = "erase_current_line"]
        unsafe fn eraseCurrentLine(self: &Document);

        #[rust_name = "erase_last_character"]
        unsafe fn eraseLastCharacter(self: &Document);

        #[rust_name = "erase_last_line"]
        unsafe fn eraseLastLine(self: &Document);

        unsafe fn end(self: &Document);

        #[rust_name = "handle_mxp_change"]
        unsafe fn handleMxpChange(self: &Document, enabled: bool);

        #[rust_name = "handle_mxp_entity"]
        unsafe fn handleMxpEntity(self: &Document, data: &str);

        #[rust_name = "handle_mxp_variable"]
        unsafe fn handleMxpVariable(self: &Document, name: &str, value: &str);

        #[rust_name = "handle_telnet_iac_ga"]
        unsafe fn handleTelnetIacGa(self: &Document);

        #[rust_name = "handle_telnet_naws"]
        unsafe fn handleTelnetNaws(self: &Document);

        #[rust_name = "handle_telnet_negotiation"]
        unsafe fn handleTelnetNegotiation(
            self: &Document,
            source: TelnetSource,
            verb: TelnetVerb,
            code: u8,
        );

        #[rust_name = "handle_telnet_subnegotiation"]
        unsafe fn handleTelnetSubnegotiation(self: &Document, code: u8, data: &QByteArray);

        #[rust_name = "permit_line"]
        unsafe fn permitLine(self: &Document, line: &str) -> bool;

        #[rust_name = "play_sound"]
        unsafe fn playSound(self: &Document, filePath: &QString);

        unsafe fn send(self: &Document, request: &SendRequest);

        #[rust_name = "send_script"]
        unsafe fn send(self: &Document, request: &SendScriptRequest);

        #[rust_name = "set_suppress_echo"]
        unsafe fn setSuppressEcho(self: &Document, suppress: bool);
    }

    extern "C++Qt" {
        include!("timekeeper.h");
        type Timekeeper;

        #[rust_name = "send_timer"]
        unsafe fn sendTimer(self: &Timekeeper, timer: &SendTimer);
        #[rust_name = "start_send_timer"]
        unsafe fn startSendTimer(self: Pin<&mut Timekeeper>, id: usize, milliseconds: u32);
    }

    extern "C++Qt" {
        include!("viewbuilder.h");

        type TableBuilder;

        #[rust_name = "set_row_count"]
        unsafe fn setRowCount(self: &TableBuilder, rows: i32);
        #[rust_name = "start_row"]
        unsafe fn startRow(self: Pin<&mut TableBuilder>, data: &QString);
        #[rust_name = "add_column"]
        unsafe fn addColumn(self: Pin<&mut TableBuilder>, text: &QString);
        #[rust_name = "add_column_bool"]
        unsafe fn addColumn(self: Pin<&mut TableBuilder>, value: bool);

        type TreeBuilder;

        #[rust_name = "start_group"]
        unsafe fn startGroup(self: Pin<&mut TreeBuilder>, text: &QString);
        #[rust_name = "start_item"]
        unsafe fn startItem(self: Pin<&mut TreeBuilder>, value: usize);
        #[rust_name = "add_column"]
        unsafe fn addColumn(self: Pin<&mut TreeBuilder>, text: &QString);
        #[rust_name = "add_column_signed"]
        unsafe fn addColumn(self: Pin<&mut TreeBuilder>, value: i64);
        #[rust_name = "add_column_unsigned"]
        unsafe fn addColumn(self: Pin<&mut TreeBuilder>, value: u64);
        #[rust_name = "add_column_floating"]
        unsafe fn addColumn(self: Pin<&mut TreeBuilder>, value: f64);
    }

    enum PluginInfo {
        Aliases,
        Author,
        Description,
        ID,
        Name,
        Purpose,
        Script,
        Sequence,
        Timers,
        Triggers,
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

    extern "RustQt" {
        #[qobject]
        type SmushClient = super::SmushClientRust;
    }

    unsafe extern "RustQt" {
        fn load_world(
            self: Pin<&mut SmushClient>,
            path: &QString,
            world: Pin<&mut World>,
        ) -> Result<()>;
        fn save_world(self: &SmushClient, path: &QString) -> Result<()>;
        fn open_log(self: Pin<&mut SmushClient>) -> Result<()>;
        fn load_plugins(self: Pin<&mut SmushClient>) -> QStringList;
        fn load_variables(self: Pin<&mut SmushClient>, path: &QString) -> Result<bool>;
        fn save_variables(self: &SmushClient, path: &QString) -> Result<bool>;
        fn populate_world(self: &SmushClient, world: Pin<&mut World>);
        fn set_world(self: Pin<&mut SmushClient>, world: &World) -> bool;
        fn palette(self: &SmushClient) -> QVector_QColor;
        fn handle_connect(self: &SmushClient, socket: Pin<&mut QTcpSocket>) -> QString;
        fn handle_disconnect(self: Pin<&mut SmushClient>);
        fn alias(
            self: Pin<&mut SmushClient>,
            command: &QString,
            source: CommandSource,
            doc: Pin<&mut Document>,
        ) -> AliasOutcomes;
        fn plugin_info(self: &SmushClient, index: usize, info_type: u8) -> QVariant;
        fn add_plugin(self: Pin<&mut SmushClient>, path: &QString) -> QString;
        fn remove_plugin(self: Pin<&mut SmushClient>, plugin_id: &QString) -> bool;
        fn build_plugins_table(self: &SmushClient, table: Pin<&mut TableBuilder>);
        fn plugin_scripts(self: &SmushClient) -> Vec<PluginPack>;
        fn read(
            self: Pin<&mut SmushClient>,
            device: Pin<&mut QTcpSocket>,
            doc: Pin<&mut Document>,
        ) -> i64;
        fn flush(self: Pin<&mut SmushClient>, doc: Pin<&mut Document>);
        fn has_output(self: &SmushClient) -> bool;
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
        fn replace_alias(self: Pin<&mut SmushClient>, index: usize, alias: &Alias) -> Result<i32>;
        fn replace_timer(
            self: Pin<&mut SmushClient>,
            index: usize,
            timer: &Timer,
            timekeeper: Pin<&mut Timekeeper>,
        ) -> i32;
        fn replace_trigger(
            self: Pin<&mut SmushClient>,
            index: usize,
            trigger: &Trigger,
        ) -> Result<i32>;
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
        unsafe fn get_variable(
            self: &SmushClient,
            index: usize,
            key: &[c_char],
            value_size: *mut usize,
        ) -> *const c_char;
        fn set_variable(
            self: Pin<&mut SmushClient>,
            index: usize,
            key: &[c_char],
            value: &[c_char],
        ) -> bool;
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
        fn stop_triggers(self: Pin<&mut SmushClient>);
    }

    unsafe impl !cxx_qt::Locking for SmushClient {}

    extern "RustQt" {
        #[qobject]
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        type Sender = super::SenderRust;
    }

    unsafe impl !cxx_qt::Locking for Sender {}

    extern "RustQt" {
        // Sender
        #[qobject]
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Timer
        #[qproperty(Occurrence, occurrence)]
        #[qproperty(QTime, at_time)]
        #[qproperty(i32, every_hour)]
        #[qproperty(i32, every_minute)]
        #[qproperty(i32, every_second)]
        #[qproperty(u32, every_millisecond)]
        #[qproperty(bool, active_closed)]
        type Timer = super::TimerRust;
    }

    unsafe impl !cxx_qt::Locking for Timer {}

    impl cxx_qt::Constructor<(), NewArguments = ()> for Timer {}
    impl cxx_qt::Constructor<(*const World, usize), NewArguments = (*const World, usize)> for Timer {}

    extern "RustQt" {
        #[qobject]
        // Sender
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Reaction
        #[qproperty(i32, sequence)]
        #[qproperty(QString, pattern)]
        #[qproperty(bool, ignore_case)]
        #[qproperty(bool, keep_evaluating)]
        #[qproperty(bool, is_regex)]
        #[qproperty(bool, expand_variables)]
        #[qproperty(bool, repeats)]
        type Reaction = super::ReactionRust;
    }

    unsafe impl !cxx_qt::Locking for Reaction {}

    extern "RustQt" {
        #[qobject]
        // Sender
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Reaction
        #[qproperty(i32, sequence)]
        #[qproperty(QString, pattern)]
        #[qproperty(bool, ignore_case)]
        #[qproperty(bool, keep_evaluating)]
        #[qproperty(bool, is_regex)]
        #[qproperty(bool, expand_variables)]
        #[qproperty(bool, repeats)]
        // Alias
        #[qproperty(bool, echo_alias)]
        #[qproperty(bool, menu)]
        #[qproperty(bool, omit_from_command_history)]
        type Alias = super::AliasRust;
    }

    unsafe impl !cxx_qt::Locking for Alias {}

    impl cxx_qt::Constructor<(), NewArguments = ()> for Alias {}
    impl cxx_qt::Constructor<(*const World, usize), NewArguments = (*const World, usize)> for Alias {}

    extern "RustQt" {
        #[qobject]
        // Sender
        #[qproperty(SendTarget, send_to)]
        #[qproperty(QString, label)]
        #[qproperty(QString, script)]
        #[qproperty(QString, group)]
        #[qproperty(QString, variable)]
        #[qproperty(bool, enabled)]
        #[qproperty(bool, one_shot)]
        #[qproperty(bool, temporary)]
        #[qproperty(bool, omit_from_output)]
        #[qproperty(bool, omit_from_log)]
        #[qproperty(QString, text)]
        // Reaction
        #[qproperty(i32, sequence)]
        #[qproperty(QString, pattern)]
        #[qproperty(bool, ignore_case)]
        #[qproperty(bool, keep_evaluating)]
        #[qproperty(bool, is_regex)]
        #[qproperty(bool, expand_variables)]
        #[qproperty(bool, repeats)]
        // Trigger
        #[qproperty(bool, change_foreground)]
        #[qproperty(QColor, foreground_color)]
        #[qproperty(bool, change_background)]
        #[qproperty(QColor, background_color)]
        #[qproperty(bool, make_bold)]
        #[qproperty(bool, make_italic)]
        #[qproperty(bool, make_underline)]
        #[qproperty(QString, sound)]
        #[qproperty(bool, sound_if_inactive)]
        #[qproperty(bool, lowercase_wildcard)]
        #[qproperty(bool, multi_line)]
        #[qproperty(i32, lines_to_match)]
        type Trigger = super::TriggerRust;
    }

    unsafe impl !cxx_qt::Locking for Trigger {}

    impl cxx_qt::Constructor<(), NewArguments = ()> for Trigger {}
    impl cxx_qt::Constructor<(*const World, usize), NewArguments = (*const World, usize)> for Trigger {}

    #[qenum(Timer)]
    enum Occurrence {
        Time,
        Interval,
    }

    extern "RustQt" {
        #[qobject]
        // IP address
        #[qproperty(QString, name)]
        #[qproperty(QString, site)]
        #[qproperty(i32, port)]
        #[qproperty(bool, use_proxy)]
        #[qproperty(QString, proxy_server)]
        #[qproperty(i32, proxy_port)]
        #[qproperty(QString, proxy_username)]
        #[qproperty(QString, proxy_password)]
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
        // Notes
        #[qproperty(QString, notes)]
        // Output
        #[qproperty(bool, show_bold)]
        #[qproperty(bool, show_italic)]
        #[qproperty(bool, show_underline)]
        #[qproperty(QString, new_activity_sound)]
        #[qproperty(bool, disable_compression)]
        #[qproperty(i32, indent_paras)]
        #[qproperty(bool, naws)]
        #[qproperty(bool, carriage_return_clears_line)]
        #[qproperty(bool, utf_8)]
        #[qproperty(bool, convert_ga_to_newline)]
        #[qproperty(QString, terminal_identification)]
        // MXP
        #[qproperty(UseMxp, use_mxp)]
        #[qproperty(QColor, hyperlink_colour)]
        #[qproperty(bool, use_custom_link_colour)]
        #[qproperty(bool, mud_can_change_link_colour)]
        #[qproperty(bool, underline_hyperlinks)]
        #[qproperty(bool, mud_can_remove_underline)]
        #[qproperty(bool, hyperlink_adds_to_command_history)]
        #[qproperty(bool, echo_hyperlink_in_output_window)]
        #[qproperty(bool, ignore_mxp_colour_changes)]
        #[qproperty(bool, send_mxp_afk_response)]
        // ANSI Color
        #[qproperty(bool, use_default_colours)]
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
        // Triggers
        #[qproperty(bool, enable_triggers)]
        // Commands
        #[qproperty(bool, display_my_input)]
        #[qproperty(QColor, echo_text_colour)]
        #[qproperty(QColor, echo_background_colour)]
        #[qproperty(bool, keep_commands_on_same_line)]
        #[qproperty(bool, no_echo_off)]
        #[qproperty(bool, enable_speed_walk)]
        #[qproperty(QString, speed_walk_prefix)]
        #[qproperty(QString, speed_walk_filler)]
        #[qproperty(f64, command_queue_delay)]
        #[qproperty(bool, enable_command_stack)]
        #[qproperty(u16, command_stack_character)]
        #[qproperty(bool, enable_spam_prevention)]
        #[qproperty(i32, spam_line_count)]
        #[qproperty(QString, spam_message)]
        // Aliases
        #[qproperty(bool, enable_aliases)]
        // Keypad
        #[qproperty(bool, numpad_enable)]
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
        #[qproperty(bool, hotkey_adds_to_command_history)]
        #[qproperty(bool, echo_hotkey_in_output_window)]
        // Scripts
        #[qproperty(QString, world_script)]
        #[qproperty(bool, enable_scripts)]
        #[qproperty(ScriptRecompile, script_reload_option)]
        #[qproperty(QColor, error_colour)]
        #[qproperty(QColor, note_text_colour)]
        type World = super::WorldRust;
    }

    unsafe extern "RustQt" {
        fn add_alias(self: Pin<&mut World>, alias: &Alias) -> QString;
        fn add_timer(self: Pin<&mut World>, timer: &Timer) -> QString;
        fn add_trigger(self: Pin<&mut World>, trigger: &Trigger) -> QString;
        fn export_aliases(self: &World) -> QString;
        fn export_timers(self: &World) -> QString;
        fn export_triggers(self: &World) -> QString;
        fn import_aliases(self: Pin<&mut World>, xml: &QString) -> QString;
        fn import_timers(self: Pin<&mut World>, xml: &QString) -> QString;
        fn import_triggers(self: Pin<&mut World>, xml: &QString) -> QString;
        fn num_aliases(self: &World) -> usize;
        fn num_timers(self: &World) -> usize;
        fn num_triggers(self: &World) -> usize;
        fn remove_alias(self: Pin<&mut World>, index: usize);
        fn remove_timer(self: Pin<&mut World>, index: usize);
        fn remove_trigger(self: Pin<&mut World>, index: usize);
        fn replace_alias(self: Pin<&mut World>, index: usize, alias: &Alias) -> QString;
        fn replace_timer(self: Pin<&mut World>, index: usize, timer: &Timer) -> QString;
        fn replace_trigger(self: Pin<&mut World>, index: usize, trigger: &Trigger) -> QString;
        fn build_alias_tree(self: &World, builder: Pin<&mut TreeBuilder>);
        fn build_timer_tree(self: &World, builder: Pin<&mut TreeBuilder>);
        fn build_trigger_tree(self: &World, builder: Pin<&mut TreeBuilder>);
    }

    #[qenum(World)]
    enum AutoConnect {
        None,
        Mush,
        Diku,
        Mxp,
    }

    #[qenum(World)]
    enum LogFormat {
        Text,
        Html,
        Raw,
    }

    #[qenum(World)]
    enum LogMode {
        Append,
        Overwrite,
    }

    #[qenum(World)]
    enum UseMxp {
        Command,
        Query,
        Always,
        Never,
    }

    #[qenum(World)]
    enum ScriptRecompile {
        Confirm,
        Always,
        Never,
    }
}
