use std::str::Utf8Error;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QVariant;
use smushclient::{OptionError, Optionable, SmushClient};
use smushclient_plugins::{Alias, PluginIndex, PluginSender, Timer, Trigger};

use crate::ffi::{self, ApiCode, SenderKind, StringView, VariableView};
use crate::get_info::InfoVisitorQVariant;
use crate::results::IntoApiCode;

#[derive(Copy, Clone)]
struct OptionIndex<'a> {
    index: PluginIndex,
    label: &'a str,
    option: &'a [u8],
}

impl<'a> OptionIndex<'a> {
    pub fn new(
        index: PluginIndex,
        label: StringView<'a>,
        option: StringView<'a>,
    ) -> Result<Self, Utf8Error> {
        Ok(Self {
            index,
            label: label.to_str()?,
            option: option.as_slice(),
        })
    }
}

impl ffi::SmushClient {
    pub fn get_sender_option(
        &self,
        kind: SenderKind,
        index: PluginIndex,
        label: StringView<'_>,
        option: StringView<'_>,
    ) -> QVariant {
        fn inner<T: PluginSender + Optionable>(client: &SmushClient, opt: OptionIndex) -> QVariant {
            let Some(sender) = client.borrow_sender::<T>(opt.index, opt.label) else {
                return QVariant::default();
            };
            sender.get_option::<InfoVisitorQVariant>(opt.option)
        }

        let Ok(opt) = OptionIndex::new(index, label, option) else {
            return QVariant::default();
        };
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => inner::<Alias>(client, opt),
            SenderKind::Timer => inner::<Timer>(client, opt),
            SenderKind::Trigger => inner::<Trigger>(client, opt),
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
    ) -> ApiCode {
        fn inner<T: PluginSender + Optionable>(
            client: &SmushClient,
            opt: OptionIndex,
            value: &[u8],
        ) -> Result<(), OptionError> {
            client
                .borrow_sender_mut::<T>(opt.index, opt.label)?
                .set_option(opt.option, value)
        }

        let Ok(opt) = OptionIndex::new(index, label, option) else {
            return kind.not_found();
        };
        let value = value.as_slice();
        let client = &self.rust().client;
        match kind {
            SenderKind::Alias => inner::<Alias>(client, opt, value).code(),
            SenderKind::Timer => inner::<Timer>(client, opt, value).code(),
            SenderKind::Trigger => inner::<Trigger>(client, opt, value).code(),
            _ => ApiCode::BadParameter,
        }
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
            .world_variant_option::<InfoVisitorQVariant>(index, option.as_slice())
    }
}
