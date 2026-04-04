use std::cell::Ref;
use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::{QString, QVariant};
use smushclient::SmushClient;
use smushclient_plugins::{
    Alias, PluginIndex, PluginSender, Reaction, RegexError, Sender, Timer, Trigger,
};

use crate::ffi::{self, ApiCode, SenderKind, StringView, VariableView};
use crate::results::{IntoCode, IntoSenderAccessCode, SenderAccessCode};

macro_rules! try_regex {
    ($i:ident) => {
        match $i.rust().try_into() {
            Ok(sender) => sender,
            Err(_) => return ApiCode::BadRegularExpression,
        }
    };
}

impl ffi::SmushClient {
    pub fn add_alias(&self, index: PluginIndex, alias: &ffi::Alias) -> ApiCode {
        self.rust()
            .client
            .add_sender::<Alias>(index, try_regex!(alias))
            .code::<Alias>()
    }

    pub fn add_timer(
        &self,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> ApiCode {
        let result = self.rust().add_timer(index, timer.rust().into());
        if let Ok(Some(timer)) = &result {
            timekeeper.start(timer);
        }
        result.code::<Timer>()
    }

    pub fn add_trigger(&self, index: PluginIndex, trigger: &ffi::Trigger) -> ApiCode {
        self.rust()
            .client
            .add_sender::<Trigger>(index, try_regex!(trigger))
            .code::<Trigger>()
    }

    pub fn add_world_alias(&self, alias: &ffi::Alias) -> ApiCode {
        self.rust()
            .client
            .add_world_sender::<Alias>(try_regex!(alias))
            .code::<Alias>()
    }

    pub fn add_world_timer(&self, timer: &ffi::Timer, timekeeper: &ffi::Timekeeper) -> ApiCode {
        let index = self.rust().world_plugin_index();
        self.add_timer(index, timer, timekeeper)
    }

    pub fn add_world_trigger(&self, trigger: &ffi::Trigger) -> ApiCode {
        self.rust()
            .client
            .add_world_sender::<Trigger>(try_regex!(trigger))
            .code::<Trigger>()
    }

    pub fn alias(
        &self,
        command: &QString,
        source: ffi::CommandSource,
        doc: Pin<&mut ffi::Document>,
    ) -> ffi::AliasOutcomes {
        let Ok(source) = source.try_into() else {
            return ffi::AliasOutcomes::new();
        };
        ffi::AliasOutcome::to_qflags(self.rust().alias(&String::from(command), source, doc))
    }

    pub fn alias_menu(&self) -> Vec<ffi::AliasMenuItem> {
        let mut menu = Vec::new();
        self.rust().client.build_alias_menu(|plugin, id, label| {
            menu.push(ffi::AliasMenuItem {
                plugin,
                id,
                text: QString::from(label),
            });
        });
        menu
    }

    pub fn finish_timer(self: Pin<&mut Self>, id: usize) -> bool {
        let result = self.rust().finish_timer(id);
        if let Some(timer) = result.timer {
            self.timer_sent(&timer);
        }
        result.done
    }

    pub fn get_alias_wildcard(
        &self,
        index: PluginIndex,
        label: StringView<'_>,
        name: StringView<'_>,
    ) -> String {
        self.get_wildcard::<Alias>(index, label, name)
            .unwrap_or_default()
    }

    pub fn get_trigger_wildcard(
        &self,
        index: PluginIndex,
        label: StringView<'_>,
        name: StringView<'_>,
    ) -> String {
        self.get_wildcard::<Trigger>(index, label, name)
            .unwrap_or_default()
    }

    pub fn invoke_alias(&self, index: PluginIndex, id: u16, doc: Pin<&mut ffi::Document>) -> bool {
        self.rust().invoke_alias(index, id, doc)
    }

    pub fn is_sender(&self, kind: SenderKind, index: PluginIndex, label: StringView<'_>) -> bool {
        self.borrow_sender(kind, index, label).is_some()
    }

    pub fn list_senders(&self, kind: SenderKind, index: PluginIndex) -> Vec<String> {
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.list_senders::<Alias>(index),
            SenderKind::Timer => client.list_senders::<Timer>(index),
            SenderKind::Trigger => client.list_senders::<Trigger>(index),
            _ => Vec::new(),
        }
    }

    pub fn poll_timers(mut self: Pin<&mut Self>) {
        for timer in self.rust().poll_timers() {
            self.as_mut().timer_sent(&timer);
        }
    }

    pub fn remove_sender(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        name: StringView<'_>,
    ) -> ApiCode {
        let Ok(name) = name.to_str() else {
            return kind.not_found();
        };
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.remove_sender::<Alias>(index, name).code::<Alias>(),
            SenderKind::Timer => client.remove_sender::<Timer>(index, name).code::<Timer>(),
            SenderKind::Trigger => client
                .remove_sender::<Trigger>(index, name)
                .code::<Trigger>(),
            _ => ApiCode::BadParameter,
        }
    }

    pub fn remove_sender_group(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        name: StringView<'_>,
    ) -> usize {
        let Ok(name) = name.to_str() else {
            return 0;
        };
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.remove_sender_group::<Alias>(index, name),
            SenderKind::Timer => client.remove_sender_group::<Timer>(index, name),
            SenderKind::Trigger => client.remove_sender_group::<Trigger>(index, name),
            _ => 0,
        }
    }

    pub fn remove_sender_groups(&self, index: PluginIndex, name: StringView<'_>) -> usize {
        let Ok(name) = name.to_str() else {
            return 0;
        };
        let client = &self.rust().client;
        client.remove_sender_group::<Alias>(index, name)
            + client.remove_sender_group::<Timer>(index, name)
            + client.remove_sender_group::<Trigger>(index, name)
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

    pub fn replace_alias(&self, index: PluginIndex, alias: &ffi::Alias) -> ApiCode {
        self.rust()
            .client
            .add_or_replace_sender::<Alias>(index, try_regex!(alias));
        ApiCode::OK
    }

    pub fn replace_timer(
        &self,
        index: PluginIndex,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> ApiCode {
        if let Some(timer) = self.rust().add_or_replace_timer(index, timer.rust().into()) {
            timekeeper.start(&timer);
        }
        ApiCode::OK
    }

    pub fn replace_trigger(&self, index: PluginIndex, trigger: &ffi::Trigger) -> ApiCode {
        self.rust()
            .client
            .add_or_replace_sender::<Trigger>(index, try_regex!(trigger));
        ApiCode::OK
    }

    pub fn replace_world_alias(&self, index: usize, alias: &ffi::Alias) -> i32 {
        self.try_replace_world_sender::<Alias, _>(index, &**alias)
            .code()
    }

    pub fn replace_world_timer(
        &self,
        index: usize,
        timer: &ffi::Timer,
        timekeeper: &ffi::Timekeeper,
    ) -> i32 {
        let (result, timer) = self
            .rust()
            .replace_world_timer(index, Timer::from(&**timer));
        if let Some(timer) = timer {
            timekeeper.start(&timer);
        }
        result.code()
    }

    pub fn replace_world_trigger(&self, index: usize, trigger: &ffi::Trigger) -> i32 {
        self.try_replace_world_sender::<Trigger, _>(index, &**trigger)
            .code()
    }

    pub fn sender_script(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
    ) -> VariableView {
        let Some(sender) = self.borrow_sender(kind, index, label) else {
            return VariableView::null();
        };
        (&sender.script).into()
    }

    pub fn sender_info(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        info_type: i64,
    ) -> QVariant {
        let Ok(label) = label.to_str() else {
            return QVariant::default();
        };
        match kind {
            SenderKind::Alias => self.rust().alias_info(index, label, info_type),
            SenderKind::Timer => self.rust().timer_info(index, label, info_type),
            SenderKind::Trigger => self.rust().trigger_info(index, label, info_type),
            _ => QVariant::default(),
        }
    }

    pub fn set_sender_enabled(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        enabled: bool,
    ) -> ApiCode {
        fn set_sender_enabled<T: PluginSender + SenderAccessCode>(
            client: &SmushClient,
            index: PluginIndex,
            label: &str,
            enabled: bool,
        ) -> ApiCode {
            client
                .set_sender_enabled::<T>(index, label, enabled)
                .code::<T>()
        }
        let Ok(label) = label.to_str() else {
            return kind.not_found();
        };
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => set_sender_enabled::<Alias>(client, index, label, enabled),
            SenderKind::Timer => set_sender_enabled::<Timer>(client, index, label, enabled),
            SenderKind::Trigger => set_sender_enabled::<Trigger>(client, index, label, enabled),
            _ => ApiCode::BadParameter,
        }
    }

    pub fn set_sender_group_enabled(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        group: StringView<'_>,
        enabled: bool,
    ) -> usize {
        let Ok(group) = group.to_str() else {
            return 0;
        };
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.set_sender_group_enabled::<Alias>(index, group, enabled),
            SenderKind::Timer => client.set_sender_group_enabled::<Timer>(index, group, enabled),
            SenderKind::Trigger => {
                client.set_sender_group_enabled::<Trigger>(index, group, enabled)
            }
            _ => 0,
        }
    }

    pub fn set_sender_groups_enabled(
        &self,
        index: PluginIndex,
        group: StringView<'_>,
        enabled: bool,
    ) -> usize {
        let Ok(group) = group.to_str() else {
            return 0;
        };
        let client = &self.rust().client;
        client.set_sender_group_enabled::<Alias>(index, group, enabled)
            + client.set_sender_group_enabled::<Timer>(index, group, enabled)
            + client.set_sender_group_enabled::<Trigger>(index, group, enabled)
    }

    pub fn simulate(&self, line: StringView<'_>, doc: Pin<&mut ffi::Document>) {
        self.rust().simulate(&line.to_string_lossy(), doc);
    }

    pub fn start_all_timers(&self, timekeeper: &ffi::Timekeeper) {
        for timer in &self.rust().start_all_timers() {
            timekeeper.start(timer);
        }
    }

    pub fn start_timers(&self, index: PluginIndex, timekeeper: &ffi::Timekeeper) {
        for timer in &self.rust().start_timers(index) {
            timekeeper.start(timer);
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

    fn borrow_sender(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
    ) -> Option<Ref<'_, Sender>> {
        fn as_sender<T: AsRef<Sender>>(sender: Ref<'_, T>) -> Ref<'_, Sender> {
            Ref::map(sender, AsRef::as_ref)
        }
        let label = label.to_str().ok()?;
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => client.borrow_sender::<Alias>(index, label).map(as_sender),
            SenderKind::Timer => client.borrow_sender::<Timer>(index, label).map(as_sender),
            SenderKind::Trigger => client.borrow_sender::<Trigger>(index, label).map(as_sender),
            _ => None,
        }
    }

    fn get_wildcard<T: PluginSender + AsRef<Reaction>>(
        &self,
        index: PluginIndex,
        label: StringView<'_>,
        name: StringView<'_>,
    ) -> Option<String> {
        self.rust()
            .client
            .get_wildcard::<T>(index, label.to_str().ok()?, name.to_str().ok()?)
    }

    fn try_replace_world_sender<'a, T, U>(
        &self,
        index: usize,
        sender: &'a U,
    ) -> Result<usize, ffi::ReplaceSenderResult>
    where
        T: PluginSender + TryFrom<&'a U, Error = RegexError>,
    {
        let sender = T::try_from(sender)?;
        let group = sender.as_ref().group.clone();
        let (i, sender) = self.client.replace_world_sender(index, sender)?;
        if sender.as_ref().group == group {
            Ok(i)
        } else {
            Err(ffi::ReplaceSenderResult::GroupChanged)
        }
    }
}
