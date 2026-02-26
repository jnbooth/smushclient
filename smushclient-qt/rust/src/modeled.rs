use std::pin::Pin;
use std::ptr;

use cxx_qt::{CxxQtType, Initialize};
use cxx_qt_lib::{QDate, QSet, QString, QVariant};
use smushclient::{SenderMap, SmushClient};
use smushclient_plugins::{
    Alias, CursorVec, Plugin, PluginIndex, PluginMetadata, PluginSender, Timer, Trigger,
};

use crate::convert::impl_constructor;
use crate::ffi;

fn data_text(data: &QVariant) -> Option<String> {
    Some(String::from(&data.value::<QString>()?))
}

pub trait Modeled {
    fn cell_text(&self, column: i32) -> QString;
    fn set_cell(&mut self, column: i32, data: &QVariant) -> Option<()>;

    fn update_in_world(
        client: &SmushClient,
        index: usize,
        column: i32,
        data: &QVariant,
    ) -> Option<usize>
    where
        Self: Clone + Eq + PluginSender,
    {
        let mut senders = client.world_senders::<Self>().borrow_mut();
        senders[index].set_cell(column, data)?;
        if senders.is_sorted() {
            return Some(index);
        }
        let clone = senders[index].clone();
        senders.sort_unstable();
        senders.iter().position(|sender| sender == &clone)
    }
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

    fn set_cell(&mut self, column: i32, data: &QVariant) -> Option<()> {
        match column {
            0 => self.label = data_text(data)?,
            1 => self.sequence = data.value()?,
            2 => {
                let text = data_text(data)?;
                self.set_pattern(text).ok()?;
            }
            3 => self.text = data_text(data)?,
            _ => return None,
        }
        Some(())
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

    fn set_cell(&mut self, column: i32, data: &QVariant) -> Option<()> {
        match column {
            0 => self.metadata.name = data_text(data)?,
            1 => self.metadata.purpose = data_text(data)?,
            2 => self.metadata.author = data_text(data)?,
            3 => self.metadata.path = data_text(data)?.into(),
            4 => self.disabled.set(!data.value()?),
            5 => self.metadata.version = data_text(data)?,
            _ => return None,
        }
        Some(())
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

    fn set_cell(&mut self, column: i32, data: &QVariant) -> Option<()> {
        match column {
            0 => self.label = data_text(data)?,
            // 1 => self.occurrence = data.value()?,
            // 2 => self.occurrence = data_text(data)?,
            3 => self.text = data_text(data)?,
            _ => return None,
        }
        Some(())
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

    fn set_cell(&mut self, column: i32, data: &QVariant) -> Option<()> {
        match column {
            0 => self.label = data_text(data)?,
            1 => self.sequence = data.value()?,
            2 => {
                let text = data_text(data)?;
                self.set_pattern(text).ok()?;
            }
            3 => self.text = data_text(data)?,
            _ => return None,
        }
        Some(())
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

impl Initialize for ffi::PluginDetails {
    fn initialize(self: core::pin::Pin<&mut Self>) {
        todo!()
    }
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

impl_constructor!(<'a>, ffi::PluginDetails, (&'a ffi::SmushClient, QString), {
    fn new((client, id): (&'a ffi::SmushClient, QString)) -> PluginDetailsRust {
        let id = String::from(&id);
        let Some(plugin) = client
            .client
            .plugins()
            .find(|plugin| plugin.metadata.id == id)
        else {
            return PluginDetailsRust::default();
        };
        PluginDetailsRust::from(&plugin.metadata)
    }
});

impl_constructor!(<'a>, ffi::PluginDetails, (&'a ffi::SmushClient, PluginIndex), {
    fn new((client, index): (&'a ffi::SmushClient, PluginIndex)) -> PluginDetailsRust {
        PluginDetailsRust::from(&client.client.plugin(index).metadata)
    }
});

fn remove_all<T: Ord>(senders: &CursorVec<T>, indices: &[usize]) {
    let mut senders_mut = senders.borrow_mut();
    for &index in indices.iter().rev() {
        senders_mut.remove(index);
    }
}

fn try_index(index: usize) -> i32 {
    index.try_into().unwrap_or(-1)
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
        let world = client.world_plugin();
        match self.sender_type {
            ffi::SenderType::Alias => world.aliases.get(index).unwrap().cell_text(column),
            ffi::SenderType::Timer => world.timers.get(index).unwrap().cell_text(column),
            ffi::SenderType::Trigger => world.triggers.get(index).unwrap().cell_text(column),
            _ => QString::default(),
        }
    }

    pub fn group_len(&self, group_index: usize) -> usize {
        self.inner.group_len(group_index)
    }

    pub fn group_index(&self, group: &str) -> i32 {
        match self.inner.group_index(group) {
            Some(index) => try_index(index),
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
            Some(i) => try_index(i),
            None => -1,
        }
    }

    pub fn recalculate(&mut self, client: &SmushClient) {
        let world = client.world_plugin();
        match self.sender_type {
            ffi::SenderType::Alias => self.inner.recalculate(&*world.aliases.borrow()),
            ffi::SenderType::Timer => self.inner.recalculate(&*world.timers.borrow()),
            ffi::SenderType::Trigger => self.inner.recalculate(&*world.triggers.borrow()),
            _ => (),
        }
    }

    fn group_indices<'a>(&'a self, group: &str, start: usize, amount: usize) -> &'a [usize] {
        let indices = &self.inner[group];
        if start >= indices.len() {
            return &[];
        }
        let end = start + amount;
        if end >= indices.len() {
            &indices[start..]
        } else {
            &indices[start..end]
        }
    }

    pub fn timer_ids(
        &self,
        client: &SmushClient,
        group: &str,
        start: usize,
        amount: usize,
    ) -> QSet<u16> {
        let mut set = QSet::default();
        let timers = &client.world_plugin().timers;
        for &index in self.group_indices(group, start, amount) {
            if let Some(timer) = timers.get(index) {
                set.insert(timer.id);
            }
        }
        set
    }

    fn remove_indices(&self, client: &SmushClient, indices: &[usize]) -> bool {
        if indices.is_empty() {
            return false;
        }
        match self.sender_type {
            ffi::SenderType::Alias => remove_all(client.world_senders::<Alias>(), indices),
            ffi::SenderType::Timer => remove_all(client.world_senders::<Timer>(), indices),
            ffi::SenderType::Trigger => remove_all(client.world_senders::<Trigger>(), indices),
            _ => return false,
        }
        true
    }

    pub fn remove(&self, client: &SmushClient, group: &str, start: usize, amount: usize) -> bool {
        let indices = self.group_indices(group, start, amount);
        self.remove_indices(client, indices)
    }

    pub fn sender_index(&self, group: &str, index: usize) -> i32 {
        match self.inner.sender_index(group, index) {
            Some(index) => try_index(index),
            None => -1,
        }
    }

    pub fn set_cell(
        &mut self,
        client: &SmushClient,
        group: &str,
        row: usize,
        column: i32,
        data: &QVariant,
    ) -> i32 {
        let Some(index) = self.inner.sender_index(group, row) else {
            return -1;
        };
        let result = match self.sender_type {
            ffi::SenderType::Alias => Alias::update_in_world(client, index, column, data),
            ffi::SenderType::Timer => Timer::update_in_world(client, index, column, data),
            ffi::SenderType::Trigger => Trigger::update_in_world(client, index, column, data),
            _ => None,
        };
        let Some(new_index) = result else {
            return -1;
        };
        if new_index == index {
            return try_index(row);
        }
        self.recalculate(client);
        let Some(new_row) = self.inner[group].iter().position(|&i| i == new_index) else {
            return -1;
        };
        try_index(new_row)
    }
}

impl_constructor!(<>, ffi::SenderMap, (ffi::SenderType,), {
    fn new((sender_type,): (ffi::SenderType,)) -> SenderMapRust {
        SenderMapRust {
            inner: SenderMap::default(),
            sender_type,
        }
    }
});

#[allow(clippy::ptr_arg)]
impl ffi::SenderMap {
    pub fn cell_text(
        &self,
        client: &ffi::SmushClient,
        group: &String,
        index: usize,
        column: i32,
    ) -> QString {
        self.rust()
            .cell_text(&client.rust().client, group, index, column)
    }

    pub fn group_len(&self, group_index: usize) -> usize {
        self.rust().group_len(group_index)
    }

    pub fn group_index(&self, group: &String) -> i32 {
        self.rust().group_index(group)
    }

    pub fn group_name(&self, group_index: usize) -> *const String {
        self.rust().group_name(group_index)
    }

    pub fn len(&self) -> usize {
        self.rust().len()
    }

    pub fn position_in_group(&self, group: &String, index: usize) -> i32 {
        self.rust().position_in_group(group, index)
    }

    pub fn recalculate(self: Pin<&mut Self>, client: &ffi::SmushClient) {
        self.rust_mut().recalculate(&client.rust().client);
    }

    pub fn remove(
        &self,
        client: &ffi::SmushClient,
        group: &String,
        start: usize,
        amount: usize,
    ) -> bool {
        self.rust()
            .remove(&client.rust().client, group, start, amount)
    }

    pub fn sender_index(&self, group: &String, index: usize) -> i32 {
        self.rust().sender_index(group, index)
    }

    pub fn set_cell(
        self: Pin<&mut Self>,
        client: &ffi::SmushClient,
        group: &String,
        index: usize,
        column: i32,
        data: &QVariant,
    ) -> i32 {
        let client = &client.rust().client;
        self.rust_mut().set_cell(client, group, index, column, data)
    }

    pub fn timer_ids(
        &self,
        client: &ffi::SmushClient,
        group: &String,
        start: usize,
        amount: usize,
    ) -> QSet<u16> {
        self.rust()
            .timer_ids(&client.rust().client, group, start, amount)
    }
}
