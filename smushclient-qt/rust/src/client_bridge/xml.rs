use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient_plugins::xml::XmlSerError;
use smushclient_plugins::{Alias, ImportError, PluginIndex, Timer, Trigger};

use crate::ffi::{self, SenderKind, StringView};

fn handle_import_error<T>(res: Result<T, ImportError>) -> Result<ffi::RegexParse, ImportError> {
    match res {
        Ok(_) => Ok(ffi::RegexParse::default()),
        Err(ImportError::Regex(e)) => Ok(e.into()),
        Err(e) => Err(e),
    }
}

impl ffi::SmushClient {
    pub fn import_xml(&self, xml: StringView<'_>) -> i64 {
        let Ok(xml) = xml.to_str() else {
            return -1;
        };
        match self.rust().client.import_xml(xml) {
            Ok(len) => len.try_into().unwrap_or(i64::MAX),
            Err(_) => -2,
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

    pub fn try_import_world(
        self: Pin<&mut Self>,
        path: &QString,
    ) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(self.rust_mut().import_world(String::from(path)))
    }

    pub fn try_import_world_aliases(&self, xml: &QString) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(
            self.rust()
                .client
                .import_world_senders::<Alias>(&String::from(xml)),
        )
    }

    pub fn try_import_world_timers(
        &self,
        xml: &QString,
        timekeeper: &ffi::Timekeeper,
    ) -> Result<ffi::RegexParse, ImportError> {
        let result = self.rust().import_world_timers(&String::from(xml));
        if let Ok(timers) = &result {
            for timer in timers {
                timekeeper.start(timer);
            }
        }
        handle_import_error(result)
    }

    pub fn try_import_world_triggers(&self, xml: &QString) -> Result<ffi::RegexParse, ImportError> {
        handle_import_error(
            self.rust()
                .client
                .import_world_senders::<Trigger>(&String::from(xml)),
        )
    }
}
