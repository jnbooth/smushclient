use std::cell::Ref;
use std::io;
use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_io::QAbstractSocket;
use cxx_qt_lib::{QString, QStringList, QVariant};
use smushclient::world::PersistError;
use smushclient::{LuaStr, SendIterable};
use smushclient_plugins::{
    Alias, LoadError, PluginIndex, RegexError, Timer, Trigger, XmlError, XmlSerError,
};

use crate::convert::Convert;
use crate::ffi::AliasOutcomes;
use crate::ffi::{self, VariableView};
use crate::get_info::InfoVisitorQVariant;
use crate::modeled::Modeled;
use crate::results::{IntoApiCode, IntoCode, IntoSenderAccessCode};

impl ffi::SmushClient {
    pub fn borrow_world_sender<T: SendIterable>(&self, index: usize) -> Option<Ref<'_, T>> {
        T::from_world(self.rust().client.world()).get(index)
    }

    pub fn load_world(self: Pin<&mut Self>, path: &QString) -> Result<(), PersistError> {
        self.rust_mut().load_world(String::from(path))
    }

    pub fn open_log(self: Pin<&mut Self>) -> io::Result<()> {
        self.rust_mut().client.open_log()
    }

    pub fn close_log(&self) -> io::Result<()> {
        self.rust().client.close_log()
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

    pub fn save_world(&self, path: &QString) -> Result<(), PersistError> {
        self.rust().save_world(String::from(path))
    }

    pub fn load_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().load_variables(String::from(path))
    }

    pub fn save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().save_variables(String::from(path))
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) -> io::Result<bool> {
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

    pub fn world_alpha_option(&self, index: PluginIndex, option: &LuaStr) -> VariableView {
        self.rust()
            .client
            .world_alpha_option(index, option)
            .unwrap_or(b"")
            .into()
    }

    pub fn world_option(&self, index: PluginIndex, option: &LuaStr) -> i32 {
        self.rust().client.world_option(index, option).unwrap_or(-1)
    }

    pub fn world_variant_option(&self, index: PluginIndex, option: &LuaStr) -> QVariant {
        self.rust()
            .client
            .world_variant_option(index, option)
            .convert()
    }

    pub fn set_world_alpha_option(
        self: Pin<&mut Self>,
        index: PluginIndex,
        option: &LuaStr,
        value: &LuaStr,
    ) -> ffi::ApiCode {
        self.rust_mut()
            .client
            .set_world_alpha_option(index, option, value.to_vec())
            .code()
    }

    pub fn set_world_option(
        self: Pin<&mut Self>,
        index: PluginIndex,
        option: &LuaStr,
        value: i32,
    ) -> ffi::ApiCode {
        self.rust_mut()
            .client
            .set_world_option(index, option, value)
            .code()
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
        let Some(plugin) = self.rust().client.plugin(index) else {
            return QString::default();
        };
        plugin.cell_text(column)
    }

    pub fn add_plugin(self: Pin<&mut Self>, path: &QString) -> Result<PluginIndex, LoadError> {
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

    pub fn reinstall_plugin(self: Pin<&mut Self>, index: PluginIndex) -> Result<usize, LoadError> {
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
        let timer = Timer::from(timer.rust());
        self.rust()
            .add_timer(index, timer, timekeeper)
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

    pub fn remove_alias(&self, index: PluginIndex, name: &LuaStr) -> ffi::ApiCode {
        self.rust()
            .client
            .remove_sender::<Alias>(index, &String::from_utf8_lossy(name))
            .code::<Alias>()
    }

    pub fn remove_timer(&self, index: PluginIndex, name: &LuaStr) -> ffi::ApiCode {
        self.rust()
            .client
            .remove_sender::<Timer>(index, &String::from_utf8_lossy(name))
            .code::<Timer>()
    }

    pub fn remove_trigger(&self, index: PluginIndex, name: &LuaStr) -> ffi::ApiCode {
        self.rust()
            .client
            .remove_sender::<Trigger>(index, &String::from_utf8_lossy(name))
            .code::<Trigger>()
    }

    pub fn remove_aliases(&self, index: PluginIndex, name: &LuaStr) -> usize {
        self.rust()
            .client
            .remove_senders::<Alias>(index, &String::from_utf8_lossy(name))
    }

    pub fn remove_timers(&self, index: PluginIndex, group: &LuaStr) -> usize {
        self.rust()
            .client
            .remove_senders::<Timer>(index, &String::from_utf8_lossy(group))
    }

    pub fn remove_triggers(&self, index: PluginIndex, group: &LuaStr) -> usize {
        self.rust()
            .client
            .remove_senders::<Trigger>(index, &String::from_utf8_lossy(group))
    }

    pub fn add_world_alias(&self, alias: &ffi::Alias) -> Result<ffi::ApiCode, RegexError> {
        let alias = Alias::try_from(alias.rust())?;
        Ok(self.rust().client.add_world_sender(alias).code::<Alias>())
    }

    pub fn add_world_timer(
        &self,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> ffi::ApiCode {
        let index = self.rust().world_plugin_index();
        self.add_timer(index, timer, timekeeper)
    }

    pub fn add_world_trigger(&self, trigger: &ffi::Trigger) -> Result<ffi::ApiCode, RegexError> {
        let trigger = Trigger::try_from(trigger.rust())?;
        Ok(self
            .rust()
            .client
            .add_world_sender(trigger)
            .code::<Trigger>())
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
        let timer = Timer::from(timer.rust());
        self.rust()
            .replace_world_timer(index, timer, timekeeper)
            .code()
    }

    pub fn replace_world_trigger(&self, index: usize, trigger: &ffi::Trigger) -> i32 {
        let Ok(trigger) = Trigger::try_from(trigger.rust()) else {
            return ffi::ReplaceSenderResult::BadRegularExpression.repr;
        };
        self.rust().replace_world_trigger(index, trigger).code()
    }

    pub fn export_world_aliases(&self) -> Result<QString, XmlSerError> {
        let xml = self.rust().client.export_world_senders::<Alias>()?;
        Ok(QString::from(&xml))
    }

    pub fn export_world_timers(&self) -> Result<QString, XmlSerError> {
        let xml = self.rust().client.export_world_senders::<Timer>()?;
        Ok(QString::from(&xml))
    }

    pub fn export_world_triggers(&self) -> Result<QString, XmlSerError> {
        let xml = self.rust().client.export_world_senders::<Trigger>()?;
        Ok(QString::from(&xml))
    }

    pub fn import_world_aliases(&self, xml: &QString) -> Result<(), XmlError> {
        self.rust()
            .client
            .import_world_senders::<Alias>(&String::from(xml))?;
        Ok(())
    }

    pub fn import_world_timers(
        &self,
        xml: &QString,
        timekeeper: &ffi::Timekeeper,
    ) -> Result<(), XmlError> {
        self.rust()
            .import_world_timers(&String::from(xml), timekeeper)
    }

    pub fn import_world_triggers(&self, xml: &QString) -> Result<(), XmlError> {
        self.rust()
            .client
            .import_world_senders::<Trigger>(&String::from(xml))?;
        Ok(())
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
        let timer = Timer::from(timer.rust());
        self.rust().add_or_replace_timer(index, timer, timekeeper);
        ffi::ApiCode::OK
    }

    pub fn replace_trigger(&self, index: PluginIndex, trigger: &ffi::Trigger) -> ffi::ApiCode {
        let Ok(trigger) = Trigger::try_from(trigger.rust()) else {
            return ffi::ApiCode::BadRegularExpression;
        };
        self.rust().client.add_or_replace_sender(index, trigger);
        ffi::ApiCode::OK
    }

    pub fn is_alias(&self, index: PluginIndex, label: &LuaStr) -> bool {
        self.rust()
            .client
            .borrow_sender::<Alias>(index, &String::from_utf8_lossy(label))
            .is_some()
    }

    pub fn is_timer(&self, index: PluginIndex, label: &LuaStr) -> bool {
        self.rust()
            .client
            .borrow_sender::<Timer>(index, &String::from_utf8_lossy(label))
            .is_some()
    }

    pub fn is_trigger(&self, index: PluginIndex, label: &LuaStr) -> bool {
        self.rust()
            .client
            .borrow_sender::<Trigger>(index, &String::from_utf8_lossy(label))
            .is_some()
    }

    pub fn set_alias_enabled(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        enabled: bool,
    ) -> ffi::ApiCode {
        self.rust()
            .client
            .set_sender_enabled::<Alias>(index, &String::from_utf8_lossy(label), enabled)
            .code::<Alias>()
    }

    pub fn set_aliases_enabled(&self, index: PluginIndex, group: &LuaStr, enabled: bool) -> bool {
        self.rust().client.set_group_enabled::<Alias>(
            index,
            &String::from_utf8_lossy(group),
            enabled,
        )
    }

    pub fn set_plugin_enabled(&self, index: PluginIndex, enabled: bool) -> bool {
        self.rust().client.set_plugin_enabled(index, enabled)
    }

    pub fn set_timer_enabled(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        enabled: bool,
    ) -> ffi::ApiCode {
        self.rust()
            .client
            .set_sender_enabled::<Timer>(index, &String::from_utf8_lossy(label), enabled)
            .code::<Timer>()
    }

    pub fn set_timers_enabled(&self, index: PluginIndex, group: &LuaStr, enabled: bool) -> bool {
        self.rust().client.set_group_enabled::<Timer>(
            index,
            &String::from_utf8_lossy(group),
            enabled,
        )
    }

    pub fn set_trigger_enabled(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        enabled: bool,
    ) -> ffi::ApiCode {
        self.rust()
            .client
            .set_sender_enabled::<Trigger>(index, &String::from_utf8_lossy(label), enabled)
            .code::<Trigger>()
    }

    pub fn set_triggers_enabled(&self, index: PluginIndex, group: &LuaStr, enabled: bool) -> bool {
        self.rust().client.set_group_enabled::<Trigger>(
            index,
            &String::from_utf8_lossy(group),
            enabled,
        )
    }

    pub fn get_alias_option(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        option: &LuaStr,
    ) -> QVariant {
        self.rust()
            .get_sender_option::<Alias>(index, &String::from_utf8_lossy(label), option)
    }

    pub fn get_timer_option(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        option: &LuaStr,
    ) -> QVariant {
        self.rust()
            .get_sender_option::<Timer>(index, &String::from_utf8_lossy(label), option)
    }

    pub fn get_trigger_option(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        option: &LuaStr,
    ) -> QVariant {
        self.rust()
            .get_sender_option::<Trigger>(index, &String::from_utf8_lossy(label), option)
    }

    pub fn set_alias_option(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        option: &LuaStr,
        value: &LuaStr,
    ) -> ffi::ApiCode {
        self.rust()
            .set_sender_option::<Alias>(index, &String::from_utf8_lossy(label), option, value)
            .code()
    }

    pub fn set_timer_option(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        option: &LuaStr,
        value: &LuaStr,
    ) -> ffi::ApiCode {
        self.rust()
            .set_sender_option::<Timer>(index, &String::from_utf8_lossy(label), option, value)
            .code()
    }

    pub fn set_trigger_option(
        &self,
        index: PluginIndex,
        label: &LuaStr,
        option: &LuaStr,
        value: &LuaStr,
    ) -> ffi::ApiCode {
        self.rust()
            .set_sender_option::<Trigger>(index, &String::from_utf8_lossy(label), option, value)
            .code()
    }

    pub fn play_buffer(&self, i: usize, buf: &[u8], volume: f32, looping: bool) -> ffi::ApiCode {
        self.rust().play_buffer(i, buf, volume, looping)
    }

    pub fn play_file(&self, i: usize, path: &LuaStr, volume: f32, looping: bool) -> ffi::ApiCode {
        self.rust()
            .play_file(i, &*String::from_utf8_lossy(path), volume, looping)
    }

    pub fn stop_sound(&self, i: usize) -> ffi::ApiCode {
        self.rust().stop_sound(i)
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

    pub fn get_variable(&self, index: PluginIndex, key: &LuaStr) -> VariableView {
        self.rust().client.borrow_variable(index, key).into()
    }

    pub fn get_metavariable(&self, key: &LuaStr) -> VariableView {
        self.rust().client.borrow_metavariable(key).into()
    }

    pub fn has_metavariable(&self, key: &LuaStr) -> bool {
        self.rust().client.has_metavariable(key)
    }

    pub fn set_variable(&self, index: PluginIndex, key: &LuaStr, value: &LuaStr) -> bool {
        self.rust()
            .client
            .set_variable(index, key.to_vec(), value.to_vec())
    }

    pub fn unset_variable(&self, index: PluginIndex, key: &LuaStr) -> bool {
        self.rust().client.unset_variable(index, key).is_some()
    }

    pub fn set_metavariable(&self, key: &LuaStr, value: &LuaStr) -> bool {
        self.rust()
            .client
            .set_metavariable(key.to_vec(), value.to_vec())
    }

    pub fn unset_metavariable(&self, key: &LuaStr) -> bool {
        self.rust().client.unset_metavariable(key).is_some()
    }

    pub fn timer_info(&self, index: PluginIndex, label: &LuaStr, info_type: u8) -> QVariant {
        self.rust()
            .timer_info(index, &String::from_utf8_lossy(label), info_type)
    }

    pub fn start_timers(&self, index: PluginIndex, timekeeper: &ffi::Timekeeper) {
        self.rust().start_timers(index, timekeeper);
    }

    pub fn start_all_timers(&self, timekeeper: &ffi::Timekeeper) {
        self.rust().start_all_timers(timekeeper);
    }

    pub fn finish_timer(&self, id: usize, timekeeper: &ffi::Timekeeper) -> bool {
        self.rust().finish_timer(id, timekeeper)
    }

    pub fn poll_timers(&self, timekeeper: &ffi::Timekeeper) {
        self.rust().poll_timers(timekeeper);
    }

    pub fn stop_senders(&self) {
        self.rust().stop_senders();
    }

    pub fn stop_aliases(&self) {
        self.rust().client.stop_evaluating::<Alias>();
    }

    pub fn stop_timers(&self) {
        self.rust().client.stop_evaluating::<Timer>();
    }

    pub fn stop_triggers(&self) {
        self.rust().client.stop_evaluating::<Trigger>();
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
