use cxx_qt::CxxQtType;
use cxx_qt_lib::QVariant;
use smushclient::{OptionError, Optionable};
use smushclient_plugins::{Alias, PluginIndex, PluginSender, Timer, Trigger};

use crate::convert::Convert;
use crate::ffi::{self, ApiCode, SenderKind, StringView, VariableView};
use crate::results::IntoApiCode;

impl ffi::SmushClient {
    pub fn get_sender_option(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        option: StringView<'_>,
    ) -> QVariant {
        match kind {
            SenderKind::Alias => self.try_get_sender_opt::<Alias>(index, label, option),
            SenderKind::Timer => self.try_get_sender_opt::<Timer>(index, label, option),
            SenderKind::Trigger => self.try_get_sender_opt::<Trigger>(index, label, option),
            _ => None,
        }
        .unwrap_or_default()
    }

    pub fn set_sender_option(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        option: StringView<'_>,
        value: StringView<'_>,
    ) -> ApiCode {
        let Ok(label) = label.to_str() else {
            return kind.not_found();
        };
        match kind {
            SenderKind::Alias => self.try_set_sender_opt::<Alias>(index, label, option, value),
            SenderKind::Timer => self.try_set_sender_opt::<Timer>(index, label, option, value),
            SenderKind::Trigger => self.try_set_sender_opt::<Trigger>(index, label, option, value),
            _ => return ApiCode::BadParameter,
        }
        .code()
    }

    pub fn set_world_alpha_option(
        &self,
        index: PluginIndex,
        option: StringView<'_>,
        value: StringView<'_>,
    ) -> ApiCode {
        let value = value.to_string_lossy().into_owned();
        self.rust()
            .client
            .set_world_alpha_option(index, option.as_slice(), value)
            .code()
    }

    pub fn set_world_option(
        &self,
        index: PluginIndex,
        option: StringView<'_>,
        value: i64,
    ) -> ApiCode {
        self.rust()
            .client
            .set_world_option(index, option.as_slice(), value)
            .code()
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

    fn try_get_sender_opt<T: PluginSender + Optionable>(
        &self,
        index: PluginIndex,
        label: StringView<'_>,
        option: StringView<'_>,
    ) -> Option<QVariant> {
        let label = label.to_str().ok()?;
        let sender = self.rust().client.borrow_sender::<T>(index, label)?;
        Some(sender.get_option(option.as_slice()).convert())
    }

    fn try_set_sender_opt<T: PluginSender + Optionable>(
        &self,
        index: PluginIndex,
        label: &str,
        option: StringView<'_>,
        value: StringView<'_>,
    ) -> Result<(), OptionError> {
        self.rust()
            .client
            .borrow_sender_mut::<T>(index, label)?
            .set_option(option.as_slice(), value.as_slice())
    }
}
