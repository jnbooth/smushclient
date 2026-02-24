use std::cell::Ref;
use std::io;
use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_io::QAbstractSocket;
use cxx_qt_lib::{QString, QStringList, QVariant};
use smushclient::SendIterable;
use smushclient::world::{LogMode, PersistError};
use smushclient_plugins::{
    Alias, ImportError, LoadError, PluginIndex, Timer, Trigger, XmlSerError,
};

use crate::convert::Convert;
use crate::ffi::{self, BytesView, SenderKind, StringView};
use crate::ffi::{AliasOutcomes, VariableView};
use crate::get_info::InfoVisitorQVariant;
use crate::modeled::Modeled;
use crate::results::{IntoApiCode, IntoCode, IntoSenderAccessCode};

fn handle_import_error<T>(res: Result<T, ImportError>) -> Result<ffi::RegexParse, ImportError> {
    match res {
        Ok(_) => Ok(ffi::RegexParse::default()),
        Err(ImportError::Regex(e)) => Ok(e.into()),
        Err(e) => Err(e),
    }
}

impl ffi::SmushClient {
    pub fn borrow_world_sender<T: SendIterable>(&self, index: usize) -> Option<Ref<'_, T>> {
        T::from_world(self.rust().client.world()).get(index)
    }

    pub fn try_load_world(self: Pin<&mut Self>, path: &QString) -> Result<(), PersistError> {
        self.rust_mut().load_world(String::from(path))
    }

    pub fn try_import_world(
        self: Pin<&mut Self>,
        path: &QString,
    ) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(self.rust_mut().import_world(String::from(path)))
    }

    pub fn try_open_log(&self) -> io::Result<()> {
        self.rust().client.open_log(String::new(), None)
    }

    pub fn try_close_log(&self) -> io::Result<()> {
        self.rust().client.close_log()
    }

    pub fn open_log(&self, path: StringView, append: bool) -> ffi::ApiCode {
        let mode = if append {
            LogMode::Append
        } else {
            LogMode::Overwrite
        };
        let client = &self.rust().client;
        if client.is_log_open() {
            ffi::ApiCode::LogFileAlreadyOpen
        } else if client.open_log(path.into(), Some(mode)).is_ok() {
            ffi::ApiCode::OK
        } else {
            ffi::ApiCode::CouldNotOpenFile
        }
    }

    pub fn close_log(&self) -> ffi::ApiCode {
        let client = &self.rust().client;
        if !client.is_log_open() {
            return ffi::ApiCode::LogFileNotOpen;
        }
        let _ = client.close_log();
        ffi::ApiCode::OK
    }

    pub fn flush_log(&self) -> ffi::ApiCode {
        let client = &self.rust().client;
        if !client.is_log_open() {
            ffi::ApiCode::LogFileNotOpen
        } else if client.flush_log().is_ok() {
            ffi::ApiCode::OK
        } else {
            ffi::ApiCode::LogFileBadWrite
        }
    }

    pub fn is_log_open(&self) -> bool {
        self.rust().client.is_log_open()
    }

    pub fn log_input(&self, note: &QString) -> ffi::ApiCode {
        let client = &self.rust().client;
        if !client.is_log_open() {
            ffi::ApiCode::LogFileNotOpen
        } else if client.log_input(&String::from(note)).is_ok() {
            ffi::ApiCode::OK
        } else {
            ffi::ApiCode::LogFileBadWrite
        }
    }

    pub fn log_note(&self, note: StringView<'_>) -> ffi::ApiCode {
        let client = &self.rust().client;
        if !client.is_log_open() {
            ffi::ApiCode::LogFileNotOpen
        } else if client.log_note(&note.to_string_lossy()).is_ok() {
            ffi::ApiCode::OK
        } else {
            ffi::ApiCode::LogFileBadWrite
        }
    }

    pub fn write_to_log(&self, note: StringView<'_>) -> ffi::ApiCode {
        let client = &self.rust().client;
        if !client.is_log_open() {
            ffi::ApiCode::LogFileNotOpen
        } else if client.write_to_log(note.as_slice()).is_ok() {
            ffi::ApiCode::OK
        } else {
            ffi::ApiCode::LogFileBadWrite
        }
    }

    pub fn load_plugins(self: Pin<&mut Self>) -> QStringList {
        self.rust_mut().load_plugins()
    }

    pub fn world_plugin_index(&self) -> PluginIndex {
        self.rust().world_plugin_index()
    }

    pub fn plugins_len(&self) -> usize {
        self.rust().client.plugins_len()
    }

    pub fn try_save_world(&self, path: &QString) -> Result<(), PersistError> {
        self.rust().save_world(String::from(path))
    }

    pub fn try_load_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().load_variables(String::from(path))
    }

    pub fn try_save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().save_variables(String::from(path))
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) -> bool {
        self.rust_mut().set_world(world.rust())
    }

    pub fn connect_to_host(&self, socket: Pin<&mut QAbstractSocket>) {
        self.rust().connect_to_host(socket);
    }

    pub fn handle_connect(&self, socket: Pin<&mut ffi::QAbstractSocket>) -> QString {
        self.rust().handle_connect(socket)
    }

    pub fn handle_disconnect(self: Pin<&mut Self>) {
        self.rust_mut().handle_disconnect();
    }

    pub fn simulate(&self, line: StringView<'_>, doc: Pin<&mut ffi::Document>) {
        self.rust().simulate(&line.to_string_lossy(), doc);
    }

    pub fn world_alpha_option(&self, index: PluginIndex, option: StringView<'_>) -> VariableView {
        self.rust()
            .client
            .world_alpha_option(index, option.as_slice())
            .into()
    }

    pub fn world_option(&self, index: PluginIndex, option: StringView<'_>) -> i64 {
        self.rust()
            .client
            .world_option(index, option.as_slice())
            .unwrap_or(-1)
    }

    pub fn world_variant_option(&self, index: PluginIndex, option: StringView<'_>) -> QVariant {
        self.rust()
            .client
            .world_variant_option(index, option.as_slice())
            .convert()
    }

    pub fn set_world_alpha_option(
        self: Pin<&mut Self>,
        index: PluginIndex,
        option: StringView<'_>,
        value: StringView<'_>,
    ) -> ffi::ApiCode {
        self.rust_mut()
            .client
            .set_world_alpha_option(index, option.as_slice(), value.to_vec())
            .code()
    }

    pub fn set_world_option(
        self: Pin<&mut Self>,
        index: PluginIndex,
        option: StringView<'_>,
        value: i64,
    ) -> ffi::ApiCode {
        self.rust_mut()
            .client
            .set_world_option(index, option.as_slice(), value)
            .code()
    }

    pub fn get_info(&self, info_type: i64) -> QVariant {
        self.rust()
            .client
            .get_info::<InfoVisitorQVariant>(info_type)
    }

    pub fn plugin_info(&self, index: PluginIndex, info_type: u8) -> QVariant {
        self.rust()
            .client
            .plugin_info::<InfoVisitorQVariant>(index, info_type)
    }

    pub fn plugin_enabled(&self, index: PluginIndex) -> bool {
        self.rust()
            .client
            .plugin(index)
            .is_some_and(|plugin| !plugin.disabled.get())
    }

    pub fn plugin_id(&self, index: PluginIndex) -> QString {
        match self.rust().client.plugin(index) {
            Some(plugin) => QString::from(&plugin.metadata.id),
            None => QString::default(),
        }
    }

    pub fn plugin_model_text(&self, index: PluginIndex, column: i32) -> QString {
        match self.rust().client.plugin(index) {
            Some(plugin) => plugin.cell_text(column),
            None => QString::default(),
        }
    }

    pub fn try_add_plugin(self: Pin<&mut Self>, path: &QString) -> Result<PluginIndex, LoadError> {
        Ok(self.rust_mut().client.add_plugin(String::from(path))?.0)
    }

    pub fn remove_plugin(self: Pin<&mut Self>, index: PluginIndex) -> bool {
        self.rust_mut().client.remove_plugin(index).is_some()
    }

    /// # Panics
    ///
    /// Panics if the index is out of bounds.
    pub fn plugin(&self, index: PluginIndex) -> ffi::PluginPack {
        self.rust().plugin(index)
    }

    pub fn reset_world_plugin(&self) {
        self.rust().reset_world_plugin();
    }

    pub fn reset_plugins(&self) -> Vec<ffi::PluginPack> {
        self.rust().reset_plugins()
    }

    pub fn try_reinstall_plugin(
        self: Pin<&mut Self>,
        index: PluginIndex,
    ) -> Result<usize, LoadError> {
        self.rust_mut().reinstall_plugin(index)
    }

    pub fn read(
        self: Pin<&mut Self>,
        device: Pin<&mut ffi::QAbstractSocket>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.rust_mut().read(device, doc)
    }

    pub fn flush(self: Pin<&mut Self>, doc: Pin<&mut ffi::Document>) {
        self.rust_mut().flush(doc);
    }

    pub fn handle_alert(&self) -> ffi::ApiCode {
        self.rust().handle_alert()
    }

    pub fn has_output(&self) -> bool {
        self.rust().client.has_output()
    }

    pub fn add_alias(&self, index: PluginIndex, alias: &ffi::Alias) -> ffi::ApiCode {
        let Ok(alias) = Alias::try_from(alias.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust().client.add_sender(index, alias).code::<Alias>()
    }

    pub fn add_timer(
        &self,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> ffi::ApiCode {
        self.rust()
            .add_timer(index, timer.rust().into(), timekeeper)
            .code::<Timer>()
    }

    pub fn add_trigger(&self, index: PluginIndex, trigger: &ffi::Trigger) -> ffi::ApiCode {
        let Ok(trigger) = Trigger::try_from(trigger.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust()
            .client
            .add_sender(index, trigger)
            .code::<Trigger>()
    }

    pub fn remove_sender(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        name: StringView<'_>,
    ) -> ffi::ApiCode {
        let client = &self.rust().client;
        let name = name.to_string_lossy();
        match kind {
            SenderKind::Alias => client.remove_sender::<Alias>(index, &name).code::<Alias>(),
            SenderKind::Timer => client.remove_sender::<Timer>(index, &name).code::<Timer>(),
            SenderKind::Trigger => client
                .remove_sender::<Trigger>(index, &name)
                .code::<Trigger>(),
            _ => ffi::ApiCode::BadParameter,
        }
    }

    pub fn remove_sender_group(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        name: StringView<'_>,
    ) -> usize {
        let client = &self.rust().client;
        let name = name.to_string_lossy();
        match kind {
            SenderKind::Alias => client.remove_sender_group::<Alias>(index, &name),
            SenderKind::Timer => client.remove_sender_group::<Timer>(index, &name),
            SenderKind::Trigger => client.remove_sender_group::<Trigger>(index, &name),
            _ => 0,
        }
    }

    pub fn remove_temporary_senders(&self, kind: SenderKind) -> usize {
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.remove_temporary_senders::<Alias>(),
            SenderKind::Timer => client.remove_temporary_senders::<Timer>(),
            SenderKind::Trigger => client.remove_temporary_senders::<Trigger>(),
            _ => 0,
        }
    }

    pub fn add_world_alias(&self, alias: &ffi::Alias) -> ffi::ApiCode {
        let Ok(alias) = Alias::try_from(alias.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust().client.add_world_sender(alias).code::<Alias>()
    }

    pub fn add_world_timer(
        &self,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> ffi::ApiCode {
        let index = self.rust().world_plugin_index();
        self.add_timer(index, timer, timekeeper)
    }

    pub fn add_world_trigger(&self, trigger: &ffi::Trigger) -> ffi::ApiCode {
        let Ok(trigger) = Trigger::try_from(trigger.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust()
            .client
            .add_world_sender(trigger)
            .code::<Trigger>()
    }

    pub fn replace_world_alias(&self, index: usize, alias: &ffi::Alias) -> i32 {
        let Ok(alias) = Alias::try_from(alias.rust()) else {
            return ffi::ReplaceSenderResult::BadRegularExpression.repr;
        };
        self.rust().replace_world_alias(index, alias).code()
    }

    pub fn replace_world_timer(
        &self,
        index: usize,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> i32 {
        self.rust()
            .replace_world_timer(index, timer.rust().into(), timekeeper)
            .code()
    }

    pub fn replace_world_trigger(&self, index: usize, trigger: &ffi::Trigger) -> i32 {
        let Ok(trigger) = Trigger::try_from(trigger.rust()) else {
            return ffi::ReplaceSenderResult::BadRegularExpression.repr;
        };
        self.rust().replace_world_trigger(index, trigger).code()
    }

    pub fn try_export_world_senders(&self, kind: SenderKind) -> Result<QString, XmlSerError> {
        let client = &self.rust().client;
        let xml = match kind {
            SenderKind::Alias => client.export_world_senders::<Alias>(),
            SenderKind::Timer => client.export_world_senders::<Timer>(),
            SenderKind::Trigger => client.export_world_senders::<Trigger>(),
            _ => return Ok(QString::default()),
        }?;
        Ok(QString::from(&xml))
    }

    pub fn try_import_world_aliases(&self, xml: &QString) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(
            self.rust()
                .client
                .import_world_senders::<Alias>(&String::from(xml)),
        )
    }

    pub fn try_import_world_timers(
        &self,
        xml: &QString,
        timekeeper: &ffi::Timekeeper,
    ) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(
            self.rust()
                .import_world_timers(&String::from(xml), timekeeper),
        )
    }

    pub fn try_import_world_triggers(&self, xml: &QString) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(
            self.rust()
                .client
                .import_world_senders::<Trigger>(&String::from(xml)),
        )
    }

    pub fn replace_alias(&self, index: PluginIndex, alias: &ffi::Alias) -> ffi::ApiCode {
        let Ok(alias) = Alias::try_from(alias.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust().client.add_or_replace_sender(index, alias);
        ffi::ApiCode::OK
    }

    pub fn replace_timer(
        &self,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> ffi::ApiCode {
        self.rust()
            .add_or_replace_timer(index, timer.rust().into(), timekeeper);
        ffi::ApiCode::OK
    }

    pub fn replace_trigger(&self, index: PluginIndex, trigger: &ffi::Trigger) -> ffi::ApiCode {
        let Ok(trigger) = Trigger::try_from(trigger.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust().client.add_or_replace_sender(index, trigger);
        ffi::ApiCode::OK
    }

    pub fn is_sender(&self, kind: SenderKind, index: PluginIndex, label: StringView<'_>) -> bool {
        let label = label.to_string_lossy();
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.borrow_sender::<Alias>(index, &label).is_some(),
            SenderKind::Timer => client.borrow_sender::<Timer>(index, &label).is_some(),
            SenderKind::Trigger => client.borrow_sender::<Trigger>(index, &label).is_some(),
            _ => false,
        }
    }

    pub fn set_sender_enabled(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        enabled: bool,
    ) -> ffi::ApiCode {
        let label = label.to_string_lossy();
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client
                .set_sender_enabled::<Alias>(index, &label, enabled)
                .code::<Alias>(),
            SenderKind::Timer => client
                .set_sender_enabled::<Timer>(index, &label, enabled)
                .code::<Timer>(),
            SenderKind::Trigger => client
                .set_sender_enabled::<Trigger>(index, &label, enabled)
                .code::<Timer>(),
            _ => ffi::ApiCode::BadParameter,
        }
    }

    pub fn set_senders_enabled(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        group: StringView<'_>,
        enabled: bool,
    ) -> bool {
        let client = &self.rust().client;
        let group = group.to_string_lossy();
        match kind {
            SenderKind::Alias => client.set_group_enabled::<Alias>(index, &group, enabled),
            SenderKind::Timer => client.set_group_enabled::<Timer>(index, &group, enabled),
            SenderKind::Trigger => client.set_group_enabled::<Trigger>(index, &group, enabled),
            _ => false,
        }
    }

    pub fn set_plugin_enabled(&self, index: PluginIndex, enabled: bool) -> bool {
        self.rust().client.set_plugin_enabled(index, enabled)
    }

    pub fn get_sender_option(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        option: StringView<'_>,
    ) -> QVariant {
        let client = self.rust();
        let label = label.to_string_lossy();
        let option = option.as_slice();
        match kind {
            SenderKind::Alias => client.get_sender_option::<Alias>(index, &label, option),
            SenderKind::Timer => client.get_sender_option::<Timer>(index, &label, option),
            SenderKind::Trigger => client.get_sender_option::<Trigger>(index, &label, option),
            _ => QVariant::default(),
        }
    }

    pub fn set_sender_option(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        option: StringView<'_>,
        value: StringView<'_>,
    ) -> ffi::ApiCode {
        let client = self.rust();
        let label = label.to_string_lossy();
        let option = option.as_slice();
        let value = value.as_slice();
        match kind {
            SenderKind::Alias => client.set_sender_option::<Alias>(index, &label, option, value),
            SenderKind::Timer => client.set_sender_option::<Timer>(index, &label, option, value),
            SenderKind::Trigger => {
                client.set_sender_option::<Trigger>(index, &label, option, value)
            }
            _ => return ffi::ApiCode::BadParameter,
        }
        .code()
    }

    pub fn play_buffer(
        &self,
        i: usize,
        buf: BytesView<'_>,
        volume: f32,
        looping: bool,
    ) -> ffi::ApiCode {
        self.client
            .play_buffer(i, buf.to_vec(), volume, looping.into())
            .code()
    }

    pub fn play_file(
        &self,
        i: usize,
        path: StringView<'_>,
        volume: f32,
        looping: bool,
    ) -> ffi::ApiCode {
        let client = &self.rust().client;
        if path.is_empty() {
            client.configure_audio_sink(i, volume, looping.into())
        } else {
            client.play_file(i, &*path.to_string_lossy(), volume, looping.into())
        }
        .code()
    }

    pub fn play_file_raw(&self, path: StringView<'_>) -> ffi::ApiCode {
        self.rust()
            .client
            .play_file_raw(&*path.to_string_lossy())
            .code()
    }

    pub fn stop_sound(&self, i: usize) -> ffi::ApiCode {
        self.rust().client.stop_sound(i).code()
    }

    pub fn ansi_note(&self, text: StringView<'_>) -> Vec<ffi::StyledSpan> {
        self.rust().ansi_note(text.as_slice())
    }

    pub fn alias(
        &self,
        command: &QString,
        source: ffi::CommandSource,
        doc: Pin<&mut ffi::Document>,
    ) -> AliasOutcomes {
        let Ok(source) = source.try_into() else {
            return AliasOutcomes::new();
        };
        ffi::AliasOutcome::to_qflags(self.rust().alias(&String::from(command), source, doc))
    }

    pub fn invoke_alias(&self, index: PluginIndex, id: u16, doc: Pin<&mut ffi::Document>) -> bool {
        self.rust().invoke_alias(index, id, doc)
    }

    pub fn alias_menu(&self) -> Vec<ffi::AliasMenuItem> {
        self.rust().alias_menu()
    }

    pub fn get_variable(&self, index: PluginIndex, key: StringView<'_>) -> VariableView {
        self.rust()
            .client
            .borrow_variable(index, key.as_slice())
            .into()
    }

    pub fn get_metavariable(&self, key: StringView<'_>) -> VariableView {
        self.rust()
            .client
            .borrow_metavariable(key.as_slice())
            .into()
    }

    pub fn has_metavariable(&self, key: StringView<'_>) -> bool {
        self.rust().client.has_metavariable(key.as_slice())
    }

    pub fn set_variable(
        &self,
        index: PluginIndex,
        key: StringView<'_>,
        value: BytesView<'_>,
    ) -> bool {
        self.rust()
            .client
            .set_variable(index, key.to_vec(), value.to_vec())
    }

    pub fn unset_variable(&self, index: PluginIndex, key: StringView<'_>) -> bool {
        self.rust()
            .client
            .unset_variable(index, key.as_slice())
            .is_some()
    }

    pub fn set_metavariable(&self, key: StringView<'_>, value: BytesView<'_>) -> bool {
        self.rust()
            .client
            .set_metavariable(key.to_vec(), value.to_vec())
    }

    pub fn unset_metavariable(&self, key: StringView<'_>) -> bool {
        self.rust()
            .client
            .unset_metavariable(key.as_slice())
            .is_some()
    }

    pub fn sender_info(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        info_type: u8,
    ) -> QVariant {
        let client = self.rust();
        let label = label.to_string_lossy();
        match kind {
            SenderKind::Alias => client
                .client
                .alias_info::<InfoVisitorQVariant>(index, &label, info_type),
            SenderKind::Timer => client.timer_info(index, &label, info_type),
            SenderKind::Trigger => client
                .client
                .trigger_info::<InfoVisitorQVariant>(index, &label, info_type),
            _ => QVariant::default(),
        }
    }

    pub fn start_timers(&self, index: PluginIndex, timekeeper: &ffi::Timekeeper) {
        self.rust().start_timers(index, timekeeper);
    }

    pub fn start_all_timers(&self, timekeeper: &ffi::Timekeeper) {
        self.rust().start_all_timers(timekeeper);
    }

    pub fn finish_timer(self: Pin<&mut Self>, id: usize) -> bool {
        let result = self.rust().finish_timer(id);
        if let Some(timer) = result.timer {
            self.timer_sent(&timer);
        }
        result.done
    }

    pub fn poll_timers(mut self: Pin<&mut Self>) {
        for timer in self.rust().poll_timers() {
            self.as_mut().timer_sent(&timer);
        }
    }

    pub fn stop_senders(&self, kind: SenderKind) {
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.stop_evaluating::<Alias>(),
            SenderKind::Timer => client.stop_evaluating::<Timer>(),
            SenderKind::Trigger => client.stop_evaluating::<Trigger>(),
            _ => (),
        }
    }

    pub fn command_splitter(&self) -> u8 {
        let world = self.rust().client.world();
        if world.enable_command_stack {
            world.command_stack_character
        } else {
            b'\n'
        }
    }
}
