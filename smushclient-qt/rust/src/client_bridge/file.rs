use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient_plugins::xml::XmlSerError;
use smushclient_plugins::{Alias, PluginIndex, Timer, Trigger};

use crate::ffi::{self, SenderKind, StringView};

impl ffi::SmushClient {
    pub fn import_world(self: Pin<&mut Self>, path: &QString) -> ffi::ParseResult {
        self.rust_mut()
            .import_world(String::from(path))
            .map(|()| 1)
            .into()
    }

    pub fn import_world_aliases(&self, xml: &QString) -> ffi::ParseResult {
        self.rust()
            .client
            .import_world_senders::<Alias>(&String::from(xml))
            .map(|result| result.len())
            .into()
    }

    pub fn import_world_timers(
        &self,
        xml: &QString,
        timekeeper: &ffi::Timekeeper,
    ) -> ffi::ParseResult {
        let result = self.rust().import_world_timers(&String::from(xml));
        if let Ok(timers) = &result {
            for timer in timers {
                timekeeper.start(timer);
            }
        }
        result.map(|result| result.len()).into()
    }

    pub fn import_world_triggers(&self, xml: &QString) -> ffi::ParseResult {
        self.rust()
            .client
            .import_world_senders::<Trigger>(&String::from(xml))
            .map(|result| result.len())
            .into()
    }

    pub fn import_xml(&self, xml: StringView<'_>) -> ffi::ParseResult {
        match xml.to_str() {
            Ok(xml) => self.rust().client.import_xml(xml).into(),
            Err(e) => ffi::ParseResult {
                target: QString::from(&*xml.to_string_lossy()),
                ..e.into()
            },
        }
    }

    pub fn try_export_xml(
        &self,
        kind: ffi::ExportKind,
        index: PluginIndex,
        name: StringView<'_>,
    ) -> Result<QString, XmlSerError> {
        let Ok(name) = name.to_str() else {
            return Ok(QString::default());
        };
        let client = &self.rust().client;
        let xml = match kind {
            ffi::ExportKind::Trigger => client.export_sender::<Trigger>(index, name),
            ffi::ExportKind::Alias => client.export_sender::<Alias>(index, name),
            ffi::ExportKind::Timer => client.export_sender::<Timer>(index, name),
            // ffi::ExportKind::Macro =>
            ffi::ExportKind::Variable => client.export_variable(index, name),
            ffi::ExportKind::Keypad => client.export_numpad_key(name),
            _ => return Ok(QString::default()),
        }?;
        Ok(QString::from(&xml))
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
}
