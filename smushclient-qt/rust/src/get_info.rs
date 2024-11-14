use std::path::Path;

use crate::ffi;
use chrono::{DateTime, NaiveDate, Utc};
use cxx_qt_lib::{QByteArray, QDate, QDateTime, QString, QVariant};
use smushclient::InfoVisitor;
use smushclient_plugins::PluginMetadata;

pub struct InfoVisitorQVariant;

impl InfoVisitor for InfoVisitorQVariant {
    type Output = QVariant;

    fn visit_bool(info: bool) -> Self::Output {
        QVariant::from(&info)
    }

    fn visit_date(info: NaiveDate) -> Self::Output {
        QVariant::from(&QDate::from(info))
    }

    fn visit_datetime(info: DateTime<Utc>) -> Self::Output {
        QVariant::from(&QDateTime::from(info))
    }

    fn visit_double(info: f64) -> Self::Output {
        QVariant::from(&info)
    }

    fn visit_i16(info: i16) -> Self::Output {
        QVariant::from(&info)
    }

    fn visit_none() -> Self::Output {
        QVariant::default()
    }

    fn visit_path(info: &Path) -> Self::Output {
        QVariant::from(&QByteArray::from(info.as_os_str().as_encoded_bytes()))
    }

    fn visit_str(info: &str) -> Self::Output {
        QVariant::from(&QString::from(info))
    }

    fn visit_usize(info: usize) -> Self::Output {
        QVariant::from(&u64::try_from(info).unwrap())
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
