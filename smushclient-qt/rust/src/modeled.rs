use crate::ffi;
use cxx_qt_lib::{QDate, QString};
use smushclient_plugins::{Plugin, PluginMetadata};

pub trait Modeled {
    fn cell_text(&self, column: i32) -> QString;
}

impl Modeled for Plugin {
    fn cell_text(&self, column: i32) -> QString {
        let metadata = &self.metadata;
        match column {
            0 => QString::from(&metadata.name),
            1 => QString::from(&metadata.purpose),
            2 => QString::from(&metadata.author),
            3 => QString::from(&*metadata.path.to_string_lossy()),
            // 4 => QString::from(&plugin.enabled.to_string()),
            5 => QString::from(&metadata.version),
            _ => QString::default(),
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct PluginDetailsRust {
    pub name: QString,
    pub version: QString,
    pub author: QString,
    pub written: QDate,
    pub modified: QDate,
    pub id: QString,
    pub file: QString,
    pub description: QString,
}

impl From<&PluginMetadata> for PluginDetailsRust {
    fn from(value: &PluginMetadata) -> Self {
        Self {
            name: QString::from(&value.name),
            version: QString::from(&value.version),
            author: QString::from(&value.author),
            written: value.written.into(),
            modified: value.modified.into(),
            id: QString::from(&value.id),
            file: QString::from(&*value.path.to_string_lossy()),
            description: QString::from(&value.description),
        }
    }
}

impl_constructor!(ffi::PluginDetails, (*const ffi::SmushClient, QString), {
    fn new((client, id): (*const ffi::SmushClient, QString)) -> PluginDetailsRust {
        let id = String::from(&id);
        let Some(plugin) = unsafe { &*client }
            .client
            .plugins()
            .find(|plugin| plugin.metadata.id == id)
        else {
            return PluginDetailsRust::default();
        };
        PluginDetailsRust::from(&plugin.metadata)
    }
});
