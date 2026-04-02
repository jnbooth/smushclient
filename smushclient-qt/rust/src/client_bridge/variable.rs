use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient::world::PersistError;
use smushclient_plugins::PluginIndex;

use crate::ffi::{self, BytesView, StringView, VariableEntry, VariableView};

impl ffi::SmushClient {
    pub fn get_metavariable(&self, key: StringView<'_>) -> VariableView {
        let Ok(key) = key.to_str() else {
            return VariableView::null();
        };
        self.rust().client.borrow_metavariable(key).into()
    }

    pub fn get_mxp_entity(&self, name: StringView<'_>) -> VariableView {
        let Ok(name) = name.to_str() else {
            return VariableView::null();
        };
        self.rust().client.borrow_mxp_entity(name).into()
    }

    pub fn get_variable(&self, index: PluginIndex, key: StringView<'_>) -> VariableView {
        let Ok(key) = key.to_str() else {
            return VariableView::null();
        };
        self.rust().client.borrow_variable(index, key).into()
    }

    pub fn has_metavariable(&self, key: StringView<'_>) -> bool {
        let Ok(key) = key.to_str() else {
            return false;
        };
        self.rust().client.has_metavariable(key)
    }

    pub fn variable_entries(&self, index: PluginIndex) -> Vec<VariableEntry> {
        let Some(variables) = self.rust().client.borrow_variables(index) else {
            return Vec::new();
        };
        variables
            .iter()
            .map(|(k, v)| VariableEntry {
                key: k.into(),
                value: v.into(),
            })
            .collect()
    }

    pub fn set_metavariable(&self, key: StringView<'_>, value: BytesView<'_>) -> bool {
        let Ok(key) = key.to_str() else {
            return false;
        };
        self.rust()
            .client
            .set_metavariable(key.to_owned(), value.to_vec());
        true
    }

    pub fn set_mxp_entity(&self, name: StringView<'_>, value: StringView<'_>) -> bool {
        let (Ok(name), Ok(value)) = (name.to_str(), value.to_str()) else {
            return false;
        };
        self.rust()
            .client
            .set_mxp_entity(name.to_owned(), value.to_owned())
    }

    pub fn set_variable(
        &self,
        index: PluginIndex,
        key: StringView<'_>,
        value: BytesView<'_>,
    ) -> bool {
        let Ok(key) = key.to_str() else {
            return false;
        };
        self.rust()
            .client
            .set_variable(index, key.to_owned(), value.to_vec());
        true
    }

    pub fn try_load_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().load_variables(String::from(path))
    }

    pub fn try_save_state(&self, index: PluginIndex, path: &QString) -> Result<(), PersistError> {
        self.rust().save_state(index, String::from(path))
    }

    pub fn try_save_variables(&self, path: &QString) -> Result<bool, PersistError> {
        self.rust().save_variables(String::from(path))
    }

    pub fn unset_metavariable(&self, key: StringView<'_>) -> bool {
        let Ok(key) = key.to_str() else {
            return false;
        };
        self.rust().client.unset_metavariable(key).is_some()
    }

    pub fn unset_variable(&self, index: PluginIndex, key: StringView<'_>) -> bool {
        let Ok(key) = key.to_str() else {
            return false;
        };
        self.rust().client.unset_variable(index, key).is_some()
    }
}
