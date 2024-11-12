use std::ffi::c_char;
use std::pin::Pin;
use std::{io, ptr};

use crate::bridge::AliasOutcomes;
use crate::ffi;
use crate::results::{IntoErrorCode, IntoResultCode};
use cxx_qt_lib::{QColor, QString, QStringList, QVariant, QVector};
use smushclient::world::PersistError;
use smushclient::{AliasBool, SendIterable, TimerBool, TriggerBool};
use smushclient_plugins::{Alias, PluginIndex, RegexError, Timer, Trigger, XmlError};

impl ffi::SmushClient {
    pub fn world_sender<T: SendIterable>(&self, index: usize) -> Option<&T> {
        T::from_world(self.cxx_qt_ffi_rust().client.world()).get(index)
    }

    pub fn load_world(
        self: Pin<&mut Self>,
        path: &QString,
        world: Pin<&mut ffi::World>,
    ) -> Result<(), PersistError> {
        *world.cxx_qt_ffi_rust_mut() = self.cxx_qt_ffi_rust_mut().load_world(path)?;
        Ok(())
    }

    pub fn open_log(self: Pin<&mut Self>) -> io::Result<()> {
        self.cxx_qt_ffi_rust_mut().client.open_log()
    }

    pub fn close_log(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut().client.close_log();
    }

    pub fn load_plugins(self: Pin<&mut Self>) -> QStringList {
        self.cxx_qt_ffi_rust_mut().load_plugins()
    }

    pub fn save_world(&self, path: &QString) -> Result<(), PersistError> {
        self.cxx_qt_ffi_rust().save_world(path)
    }

    pub fn load_variables(self: Pin<&mut Self>, path: &QString) -> Result<bool, PersistError> {
        self.cxx_qt_ffi_rust_mut().load_variables(path)
    }

    pub fn save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.cxx_qt_ffi_rust().save_variables(path)
    }

    pub fn populate_world(&self, world: Pin<&mut ffi::World>) {
        self.cxx_qt_ffi_rust()
            .populate_world(&mut world.cxx_qt_ffi_rust_mut());
    }

    pub fn set_world(self: Pin<&mut Self>, world: &ffi::World) -> bool {
        return self
            .cxx_qt_ffi_rust_mut()
            .set_world(world.cxx_qt_ffi_rust());
    }

    pub fn palette(&self) -> QVector<QColor> {
        QVector::from(&self.cxx_qt_ffi_rust().palette())
    }

    pub fn handle_connect(&self, socket: Pin<&mut ffi::QTcpSocket>) -> QString {
        self.cxx_qt_ffi_rust().handle_connect(socket.into())
    }

    pub fn handle_disconnect(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut().handle_disconnect();
    }

    pub fn plugin_info(&self, index: PluginIndex, info_type: u8) -> QVariant {
        self.cxx_qt_ffi_rust().plugin_info(index, info_type)
    }

    pub fn build_plugins_table(&self, table: Pin<&mut ffi::TableBuilder>) {
        self.cxx_qt_ffi_rust().build_plugins_table(table.into());
    }

    pub fn build_aliases_tree(&self, tree: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust()
            .build_senders_tree::<Alias>(tree.into());
    }

    pub fn build_timers_tree(&self, tree: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust()
            .build_senders_tree::<Timer>(tree.into());
    }

    pub fn build_triggers_tree(&self, tree: Pin<&mut ffi::TreeBuilder>) {
        self.cxx_qt_ffi_rust()
            .build_senders_tree::<Trigger>(tree.into());
    }

    pub fn world_aliases_len(&self) -> usize {
        self.cxx_qt_ffi_rust().client.world().aliases.len()
    }

    pub fn world_timers_len(&self) -> usize {
        self.cxx_qt_ffi_rust().client.world().timers.len()
    }

    pub fn world_triggers_len(&self) -> usize {
        self.cxx_qt_ffi_rust().client.world().triggers.len()
    }

    pub fn add_plugin(self: Pin<&mut Self>, path: &QString) -> QString {
        match self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_plugin(String::from(path))
        {
            Ok(_) => QString::default(),
            Err(e) => QString::from(&e.to_string()),
        }
    }

    pub fn remove_plugin(self: Pin<&mut Self>, plugin_id: &QString) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_plugin(&String::from(plugin_id))
            .is_some()
    }

    pub fn plugin_scripts(&self) -> Vec<ffi::PluginPack> {
        self.cxx_qt_ffi_rust().plugin_scripts()
    }

    pub fn read(
        self: Pin<&mut Self>,
        device: Pin<&mut ffi::QTcpSocket>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.cxx_qt_ffi_rust_mut().read(device.into(), doc.into())
    }

    pub fn flush(self: Pin<&mut Self>, doc: Pin<&mut ffi::Document>) {
        self.cxx_qt_ffi_rust_mut().flush(doc.into());
    }

    pub fn has_output(&self) -> bool {
        self.cxx_qt_ffi_rust().client.has_output()
    }

    pub fn add_alias(
        self: Pin<&mut Self>,
        index: PluginIndex,
        alias: &ffi::Alias,
    ) -> Result<i32, RegexError> {
        let alias = Alias::try_from(alias.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_sender(index, alias)
            .code())
    }

    pub fn add_timer(
        self: Pin<&mut Self>,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> i32 {
        let timer = Timer::from(timer.cxx_qt_ffi_rust());
        self.cxx_qt_ffi_rust_mut()
            .add_timer(index, timer, timekeeper.into())
            .code()
    }

    pub fn add_trigger(
        self: Pin<&mut Self>,
        index: PluginIndex,
        trigger: &ffi::Trigger,
    ) -> Result<i32, RegexError> {
        let trigger = Trigger::try_from(trigger.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_sender(index, trigger)
            .code())
    }

    pub fn remove_alias(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_sender::<Alias>(index, &String::from(name))
            .code()
    }

    pub fn remove_timer(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_sender::<Timer>(index, &String::from(name))
            .code()
    }

    pub fn remove_trigger(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_sender::<Trigger>(index, &String::from(name))
            .code()
    }

    pub fn remove_aliases(self: Pin<&mut Self>, index: PluginIndex, name: &QString) -> usize {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_senders::<Alias>(index, &String::from(name))
    }

    pub fn remove_timers(self: Pin<&mut Self>, index: PluginIndex, group: &QString) -> usize {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_senders::<Timer>(index, &String::from(group))
    }

    pub fn remove_triggers(self: Pin<&mut Self>, index: PluginIndex, group: &QString) -> usize {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_senders::<Trigger>(index, &String::from(group))
    }

    pub fn add_world_alias(self: Pin<&mut Self>, alias: &ffi::Alias) -> Result<i32, RegexError> {
        let alias = Alias::try_from(alias.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_world_sender(alias)
            .code())
    }

    pub fn add_world_timer(
        self: Pin<&mut Self>,
        timer: &ffi::Timer,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> i32 {
        let index = self.cxx_qt_ffi_rust().world_plugin_index();
        self.add_timer(index, timer, timekeeper)
    }

    pub fn add_world_trigger(
        self: Pin<&mut Self>,
        trigger: &ffi::Trigger,
    ) -> Result<i32, RegexError> {
        let trigger = Trigger::try_from(trigger.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .add_world_sender(trigger)
            .code())
    }

    pub fn replace_world_alias(
        self: Pin<&mut Self>,
        index: usize,
        alias: &ffi::Alias,
    ) -> Result<i32, RegexError> {
        let alias = Alias::try_from(alias.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .replace_world_sender(index, alias)
            .code())
    }

    pub fn replace_world_timer(
        self: Pin<&mut Self>,
        index: usize,
        timer: &ffi::Timer,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> i32 {
        let timer = Timer::from(timer.cxx_qt_ffi_rust());
        self.cxx_qt_ffi_rust_mut()
            .replace_world_timer(index, timer, timekeeper.into())
            .code()
    }

    pub fn replace_world_trigger(
        self: Pin<&mut Self>,
        index: usize,
        trigger: &ffi::Trigger,
    ) -> Result<i32, RegexError> {
        let trigger = Trigger::try_from(trigger.cxx_qt_ffi_rust())?;
        Ok(self
            .cxx_qt_ffi_rust_mut()
            .client
            .replace_world_sender(index, trigger)
            .code())
    }

    pub fn remove_world_alias(self: Pin<&mut Self>, i: usize) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_world_sender::<Alias>(i)
            .is_ok()
    }

    pub fn remove_world_timer(self: Pin<&mut Self>, i: usize) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_world_sender::<Timer>(i)
            .is_ok()
    }

    pub fn remove_world_trigger(self: Pin<&mut Self>, i: usize) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .remove_world_sender::<Trigger>(i)
            .is_ok()
    }

    pub fn export_world_aliases(&self) -> Result<QString, XmlError> {
        let xml = self
            .cxx_qt_ffi_rust()
            .client
            .export_world_senders::<Alias>()?;
        Ok(QString::from(&xml))
    }

    pub fn export_world_timers(&self) -> Result<QString, XmlError> {
        let xml = self
            .cxx_qt_ffi_rust()
            .client
            .export_world_senders::<Timer>()?;
        Ok(QString::from(&xml))
    }

    pub fn export_world_triggers(&self) -> Result<QString, XmlError> {
        let xml = self
            .cxx_qt_ffi_rust()
            .client
            .export_world_senders::<Trigger>()?;
        Ok(QString::from(&xml))
    }

    pub fn import_world_aliases(self: Pin<&mut Self>, xml: &QString) -> Result<(), XmlError> {
        self.cxx_qt_ffi_rust_mut()
            .client
            .import_world_senders::<Alias>(&String::from(xml))?;
        Ok(())
    }

    pub fn import_world_timers(
        self: Pin<&mut Self>,
        xml: &QString,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> Result<(), XmlError> {
        self.cxx_qt_ffi_rust_mut()
            .import_world_timers(xml, timekeeper.into())
    }

    pub fn import_world_triggers(self: Pin<&mut Self>, xml: &QString) -> Result<(), XmlError> {
        self.cxx_qt_ffi_rust_mut()
            .client
            .import_world_senders::<Trigger>(&String::from(xml))?;
        Ok(())
    }

    pub fn replace_alias(
        self: Pin<&mut Self>,
        index: PluginIndex,
        alias: &ffi::Alias,
    ) -> Result<(), RegexError> {
        let alias = Alias::try_from(alias.cxx_qt_ffi_rust())?;
        self.cxx_qt_ffi_rust_mut()
            .client
            .add_or_replace_sender(index, alias);
        Ok(())
    }

    pub fn replace_timer(
        self: Pin<&mut Self>,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) {
        let timer = Timer::from(timer.cxx_qt_ffi_rust());
        self.cxx_qt_ffi_rust_mut()
            .add_or_replace_timer(index, timer, timekeeper.into());
    }

    pub fn replace_trigger(
        self: Pin<&mut Self>,
        index: PluginIndex,
        trigger: &ffi::Trigger,
    ) -> Result<(), RegexError> {
        let trigger = Trigger::try_from(trigger.cxx_qt_ffi_rust())?;
        self.cxx_qt_ffi_rust_mut()
            .client
            .add_or_replace_sender(index, trigger);
        Ok(())
    }

    pub fn is_alias(&self, index: PluginIndex, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .find_sender::<Alias>(index, &String::from(label))
            .is_some()
    }

    pub fn is_timer(&self, index: PluginIndex, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .find_sender::<Timer>(index, &String::from(label))
            .is_some()
    }

    pub fn is_trigger(&self, index: PluginIndex, label: &QString) -> bool {
        self.cxx_qt_ffi_rust()
            .client
            .find_sender::<Trigger>(index, &String::from(label))
            .is_some()
    }

    pub fn set_alias_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        enabled: bool,
    ) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Alias>(index, &String::from(label), enabled)
            .code()
    }

    pub fn set_aliases_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        group: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Alias>(index, &String::from(group), enabled)
    }

    pub fn set_plugin_enabled(self: Pin<&mut Self>, index: PluginIndex, enabled: bool) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_plugin_enabled(index, enabled)
    }

    pub fn set_timer_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        enabled: bool,
    ) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Timer>(index, &String::from(label), enabled)
            .code()
    }

    pub fn set_timers_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        group: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Timer>(index, &String::from(group), enabled)
    }

    pub fn set_trigger_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        enabled: bool,
    ) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_sender_enabled::<Trigger>(index, &String::from(label), enabled)
            .code()
    }

    pub fn set_triggers_enabled(
        self: Pin<&mut Self>,
        index: PluginIndex,
        group: &QString,
        enabled: bool,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_group_enabled::<Trigger>(index, &String::from(group), enabled)
    }

    pub fn set_alias_bool(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        option: ffi::AliasBool,
        value: bool,
    ) -> i32 {
        let Ok(option) = AliasBool::try_from(option) else {
            return ffi::SenderAccessResult::BadParameter.repr;
        };
        self.cxx_qt_ffi_rust_mut()
            .set_bool(index, label, option, value)
            .code()
    }

    pub fn set_timer_bool(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        option: ffi::TimerBool,
        value: bool,
    ) -> i32 {
        let Ok(option) = TimerBool::try_from(option) else {
            return -3;
        };
        self.cxx_qt_ffi_rust_mut()
            .set_bool(index, label, option, value)
            .code()
    }

    pub fn set_trigger_bool(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        option: ffi::TriggerBool,
        value: bool,
    ) -> i32 {
        let Ok(option) = TriggerBool::try_from(option) else {
            return -3;
        };
        self.cxx_qt_ffi_rust_mut()
            .set_bool(index, label, option, value)
            .code()
    }

    pub fn set_trigger_group(
        self: Pin<&mut Self>,
        index: PluginIndex,
        label: &QString,
        group: &QString,
    ) -> i32 {
        self.cxx_qt_ffi_rust_mut()
            .set_sender_group::<Trigger>(index, label, group)
            .code()
    }

    pub fn alias(
        self: Pin<&mut Self>,
        command: &QString,
        source: ffi::CommandSource,
        doc: Pin<&mut ffi::Document>,
    ) -> AliasOutcomes {
        let Ok(source) = source.try_into() else {
            return AliasOutcomes(0);
        };
        self.cxx_qt_ffi_rust_mut()
            .alias(command, source, doc.into())
    }

    /// # Safety
    ///
    /// Refer to the safety documentation for [`std::slice::from_raw_parts`].
    pub unsafe fn get_variable(
        &self,
        index: PluginIndex,
        key: &[c_char],
        value_size: *mut usize,
    ) -> *const c_char {
        let Some(value) = self.cxx_qt_ffi_rust().client.get_variable(index, key) else {
            return ptr::null();
        };
        if !value_size.is_null() {
            *value_size = value.len();
        }
        value.as_ptr()
    }

    pub fn set_variable(
        self: Pin<&mut Self>,
        index: PluginIndex,
        key: &[c_char],
        value: &[c_char],
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .client
            .set_variable(index, key.to_vec(), value.to_vec())
    }

    pub fn start_timers(
        self: Pin<&mut Self>,
        index: PluginIndex,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) {
        self.cxx_qt_ffi_rust_mut()
            .start_timers(index, timekeeper.into());
    }

    pub fn finish_timer(
        self: Pin<&mut Self>,
        id: usize,
        timekeeper: Pin<&mut ffi::Timekeeper>,
    ) -> bool {
        self.cxx_qt_ffi_rust_mut()
            .finish_timer(id, timekeeper.into())
    }

    pub fn poll_timers(self: Pin<&mut Self>, timekeeper: Pin<&mut ffi::Timekeeper>) {
        self.cxx_qt_ffi_rust_mut().poll_timers(timekeeper.into());
    }

    pub fn stop_senders(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut().stop_senders();
    }

    pub fn stop_aliases(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut().client.stop_evaluating::<Alias>();
    }

    pub fn stop_timers(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut().client.stop_evaluating::<Timer>();
    }

    pub fn stop_triggers(self: Pin<&mut Self>) {
        self.cxx_qt_ffi_rust_mut()
            .client
            .stop_evaluating::<Trigger>();
    }
}