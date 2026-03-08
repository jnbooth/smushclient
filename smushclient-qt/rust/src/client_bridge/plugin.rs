use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::{QString, QStringList, QVariant};
use smushclient_plugins::{LoadError, PluginIndex};

use crate::ffi;
use crate::get_info::InfoVisitorQVariant;
use crate::modeled::Modeled;

impl ffi::SmushClient {
    pub fn load_plugins(self: Pin<&mut Self>) -> QStringList {
        self.rust_mut().load_plugins()
    }

    /// # Panics
    ///
    /// Panics if the index is out of bounds.
    #[track_caller]
    pub fn plugin(&self, index: PluginIndex) -> ffi::PluginPack {
        self.rust().client.plugin(index).into()
    }

    pub fn plugin_enabled(&self, index: PluginIndex) -> bool {
        !self.rust().client.plugin(index).disabled.get()
    }

    pub fn plugin_id(&self, index: PluginIndex) -> QString {
        QString::from(&self.rust().client.plugin(index).metadata.id)
    }

    pub fn plugin_info(&self, index: PluginIndex, info_type: i64) -> QVariant {
        self.rust()
            .client
            .plugin_info::<InfoVisitorQVariant>(index, info_type)
    }

    pub fn plugins_len(&self) -> usize {
        self.rust().client.plugins_len()
    }

    pub fn plugin_model_text(&self, index: PluginIndex, column: i32) -> QString {
        self.rust().client.plugin(index).cell_text(column)
    }

    pub fn remove_plugin(self: Pin<&mut Self>, index: PluginIndex) -> bool {
        self.rust_mut().client.remove_plugin(index).is_some()
    }

    pub fn reset_plugins(&self) -> Vec<ffi::PluginPack> {
        self.rust().reset_plugins()
    }

    pub fn reset_world_plugin(&self) {
        self.rust().reset_world_plugin();
    }

    pub fn set_plugin_enabled(&self, index: usize, enable: bool) {
        self.rust().client.set_plugin_enabled(index, enable);
    }

    pub fn try_add_plugin(self: Pin<&mut Self>, path: &QString) -> Result<PluginIndex, LoadError> {
        Ok(self.rust_mut().client.add_plugin(String::from(path))?.0)
    }

    pub fn try_reinstall_plugin(
        self: Pin<&mut Self>,
        index: PluginIndex,
    ) -> Result<usize, LoadError> {
        self.rust_mut().reinstall_plugin(index)
    }

    /// # Panics
    ///
    /// Panics if there is no world plugin.
    #[track_caller]
    pub fn world_plugin_index(&self) -> PluginIndex {
        self.rust().world_plugin_index()
    }
}
