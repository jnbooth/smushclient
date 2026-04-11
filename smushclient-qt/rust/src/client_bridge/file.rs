use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient_plugins::xml::{XmlIterable, XmlSerError};
use smushclient_plugins::{Alias, PluginIndex, Timer, Trigger};

use crate::ffi::{self, ExportKind, SenderKind, StringView};

macro_rules! try_xml {
    ($e:expr) => {
        match XmlIterable::from_xml_list_str(&String::from($e)).collect() {
            Ok(senders) => senders,
            Err(e) => return e.into(),
        }
    };
}

impl ffi::SmushClient {
    pub fn import_world(self: Pin<&mut Self>, path: &QString) -> ffi::ParseResult {
        self.rust_mut()
            .import_world(String::from(path))
            .map(|()| 1)
            .into()
    }

    pub fn import_world_aliases(&self, xml: &QString) -> ffi::ParseResult {
        let aliases: Vec<Alias> = try_xml!(xml);
        self.rust()
            .client
            .world_plugin()
            .import_senders(aliases)
            .into()
    }

    pub fn import_world_timers(
        &self,
        xml: &QString,
        timekeeper: &ffi::Timekeeper,
    ) -> ffi::ParseResult {
        let timers: Vec<Timer> = try_xml!(xml);
        let rust = self.rust();
        for timer in rust.import_world_timers(&timers) {
            timekeeper.start(&timer);
        }
        rust.client.world_plugin().import_senders(timers).into()
    }

    pub fn import_world_triggers(&self, xml: &QString) -> ffi::ParseResult {
        let triggers: Vec<Trigger> = try_xml!(xml);
        self.rust()
            .client
            .world_plugin()
            .import_senders(triggers)
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
        kind: ExportKind,
        index: PluginIndex,
        name: StringView<'_>,
    ) -> Result<QString, XmlSerError> {
        let Ok(name) = name.to_str() else {
            return Ok(QString::default());
        };
        let client = &self.rust().client;
        let xml = match kind {
            ExportKind::Trigger => client.export_sender::<Trigger>(index, name),
            ExportKind::Alias => client.export_sender::<Alias>(index, name),
            ExportKind::Timer => client.export_sender::<Timer>(index, name),
            // ExportKind::Macro =>
            ExportKind::Variable => client.export_variable(index, name),
            ExportKind::Keypad => client.export_numpad_key(name),
            _ => return Ok(QString::default()),
        }?;
        Ok(QString::from(&xml))
    }

    pub fn try_export_world_senders(&self, kind: ffi::SenderKind) -> Result<QString, XmlSerError> {
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
