use std::pin::Pin;
use std::ptr;

use crate::ffi;
use cxx_qt_lib::{QDate, QString};
use smushclient::{SenderMap, SmushClient};
use smushclient_plugins::{Alias, CursorVec, Plugin, PluginMetadata, Timer, Trigger};

pub trait Modeled {
    fn cell_text(&self, column: i32) -> QString;
}

impl Modeled for Alias {
    fn cell_text(&self, column: i32) -> QString {
        match column {
            0 => QString::from(&self.label),
            1 => QString::from(&self.sequence.to_string()),
            2 => QString::from(&self.pattern),
            3 => QString::from(&self.text),
            _ => QString::default(),
        }
    }
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

impl Modeled for Timer {
    fn cell_text(&self, column: i32) -> QString {
        match column {
            0 => QString::from(&self.label),
            // 1 => QString::From(match &self.occurence {}),
            2 => QString::from(&self.occurrence.to_string()),
            3 => QString::from(&self.text),
            _ => QString::default(),
        }
    }
}

impl Modeled for Trigger {
    fn cell_text(&self, column: i32) -> QString {
        match column {
            0 => QString::from(&self.label),
            1 => QString::from(&self.sequence.to_string()),
            2 => QString::from(&self.pattern),
            3 => QString::from(&self.text),
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

fn remove_all<T: Ord>(senders: &mut CursorVec<T>, indices: &[usize]) {
    for &index in indices.iter().rev() {
        senders.remove(index);
    }
}

#[derive(Clone, PartialEq, Eq)]
pub struct SenderMapRust {
    inner: SenderMap,
    sender_type: ffi::SenderType,
}

impl SenderMapRust {
    pub fn cell_text(
        &self,
        client: &SmushClient,
        group: &str,
        index: usize,
        column: i32,
    ) -> QString {
        let Some(index) = self.inner.sender_index(group, index) else {
            return QString::default();
        };
        let world = client.world();
        match self.sender_type {
            ffi::SenderType::Alias => world.aliases[index].cell_text(column),
            ffi::SenderType::Timer => world.timers[index].cell_text(column),
            ffi::SenderType::Trigger => world.triggers[index].cell_text(column),
            _ => QString::default(),
        }
    }

    pub fn group_len(&self, group_index: usize) -> usize {
        self.inner.group_len(group_index)
    }

    pub fn group_index(&self, group: &str) -> i32 {
        match self.inner.group_index(group) {
            Some(index) => i32::try_from(index).unwrap_or(-1),
            None => -1,
        }
    }

    pub fn group_name(&self, group_index: usize) -> *const String {
        match self.inner.group_name(group_index) {
            Some(name) => ptr::from_ref(name),
            None => ptr::null(),
        }
    }

    pub fn len(&self) -> usize {
        self.inner.len()
    }

    pub fn position_in_group(&self, group: &str, index: usize) -> i32 {
        match self.inner[group].iter().position(|&i| i == index) {
            Some(i) => i32::try_from(i).unwrap_or(-1),
            None => -1,
        }
    }

    pub fn recalculate(&mut self, client: &SmushClient) {
        let world = client.world();
        match self.sender_type {
            ffi::SenderType::Alias => self.inner.recalculate(&world.aliases),
            ffi::SenderType::Timer => self.inner.recalculate(&world.timers),
            ffi::SenderType::Trigger => self.inner.recalculate(&world.triggers),
            _ => (),
        }
    }

    pub fn remove(
        &self,
        client: &mut SmushClient,
        group: &str,
        start: usize,
        amount: usize,
    ) -> bool {
        let indices = &self.inner[group];
        if start >= indices.len() {
            return false;
        }
        let end = start + amount;
        let indices = if end >= indices.len() {
            &indices[start..]
        } else {
            &indices[start..end]
        };
        if indices.is_empty() {
            return false;
        }
        match self.sender_type {
            ffi::SenderType::Alias => remove_all(client.world_senders_mut::<Alias>(), indices),
            ffi::SenderType::Timer => remove_all(client.world_senders_mut::<Timer>(), indices),
            ffi::SenderType::Trigger => remove_all(client.world_senders_mut::<Trigger>(), indices),
            _ => return false,
        }
        true
    }

    pub fn sender_index(&self, group: &str, index: usize) -> i32 {
        match self.inner.sender_index(group, index) {
            Some(index) => i32::try_from(index).unwrap_or(-1),
            None => -1,
        }
    }
}

impl_constructor!(ffi::SenderMap, (ffi::SenderType,), {
    fn new((sender_type,): (ffi::SenderType,)) -> SenderMapRust {
        SenderMapRust {
            inner: SenderMap::default(),
            sender_type,
        }
    }
});

impl ffi::SenderMap {
    #[allow(clippy::ptr_arg)]
    pub fn cell_text(
        &self,
        client: &ffi::SmushClient,
        group: &String,
        index: usize,
        column: i32,
    ) -> QString {
        self.cxx_qt_ffi_rust()
            .cell_text(&client.cxx_qt_ffi_rust().client, group, index, column)
    }

    pub fn group_len(&self, group_index: usize) -> usize {
        self.cxx_qt_ffi_rust().group_len(group_index)
    }

    #[allow(clippy::ptr_arg)]
    pub fn group_index(&self, group: &String) -> i32 {
        self.cxx_qt_ffi_rust().group_index(group)
    }

    pub fn group_name(&self, group_index: usize) -> *const String {
        self.cxx_qt_ffi_rust().group_name(group_index)
    }

    pub fn len(&self) -> usize {
        self.cxx_qt_ffi_rust().len()
    }

    #[allow(clippy::ptr_arg)]
    pub fn position_in_group(&self, group: &String, index: usize) -> i32 {
        self.cxx_qt_ffi_rust().position_in_group(group, index)
    }

    pub fn recalculate(self: Pin<&mut Self>, client: &ffi::SmushClient) {
        self.cxx_qt_ffi_rust_mut()
            .recalculate(&client.cxx_qt_ffi_rust().client);
    }

    #[allow(clippy::ptr_arg)]
    pub fn remove(
        &self,
        client: Pin<&mut ffi::SmushClient>,
        group: &String,
        start: usize,
        amount: usize,
    ) -> bool {
        self.cxx_qt_ffi_rust().remove(
            &mut client.cxx_qt_ffi_rust_mut().client,
            group,
            start,
            amount,
        )
    }

    #[allow(clippy::ptr_arg)]
    pub fn sender_index(&self, group: &String, index: usize) -> i32 {
        self.cxx_qt_ffi_rust().sender_index(group, index)
    }
}
