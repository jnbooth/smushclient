use std::pin::Pin;
use std::ptr;

use crate::convert::impl_constructor;
use crate::ffi;
use cxx_qt::CxxQtType;
use cxx_qt_lib::{QDate, QSet, QString, QVariant};
use smushclient::{SendIterable, SenderMap, SmushClient};
use smushclient_plugins::{Alias, CursorVec, Plugin, PluginMetadata, Reaction, Timer, Trigger};

fn data_text(data: &QVariant) -> Option<String> {
    Some(String::from(&data.value::<QString>()?))
}

pub trait Modeled {
    fn cell_text(&self, column: i32) -> QString;
    fn set_cell(&mut self, column: i32, data: &QVariant) -> Option<()>;

    fn update_in_world(
        client: &mut SmushClient,
        index: usize,
        column: i32,
        data: &QVariant,
    ) -> Option<usize>
    where
        Self: Clone + Eq + SendIterable,
    {
        let senders = client.world_senders_mut::<Self>();
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
                let regex = Reaction::make_regex(&text, self.is_regex).ok()?;
                self.pattern = text;
                self.regex = regex;
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
            4 => self.disabled = !data.value()?,
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
                let regex = Reaction::make_regex(&text, self.is_regex).ok()?;
                self.pattern = text;
                self.regex = regex;
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
        // SAFETY: The C++ caller must pass a valid pointer.
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
        let timers = &client.world().timers;
        for &index in self.group_indices(group, start, amount) {
            if let Some(timer) = timers.get(index) {
                set.insert(timer.id);
            }
        }
        set
    }

    fn remove_indices(&self, client: &mut SmushClient, indices: &[usize]) -> bool {
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

    pub fn remove(
        &self,
        client: &mut SmushClient,
        group: &str,
        start: usize,
        amount: usize,
    ) -> bool {
        let indices = self.group_indices(group, start, amount);
        self.remove_indices(client, indices)
    }

    pub fn sender_index(&self, group: &str, index: usize) -> i32 {
        match self.inner.sender_index(group, index) {
            Some(index) => i32::try_from(index).unwrap_or(-1),
            None => -1,
        }
    }

    pub fn set_cell(
        &mut self,
        client: &mut SmushClient,
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
            return i32::try_from(row).unwrap_or(-1);
        }
        self.recalculate(client);
        let Some(new_row) = self.inner[group].iter().position(|&i| i == new_index) else {
            return -1;
        };
        i32::try_from(new_row).unwrap_or(-1)
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
        client: Pin<&mut ffi::SmushClient>,
        group: &String,
        start: usize,
        amount: usize,
    ) -> bool {
        self.rust()
            .remove(&mut client.rust_mut().client, group, start, amount)
    }

    pub fn sender_index(&self, group: &String, index: usize) -> i32 {
        self.rust().sender_index(group, index)
    }

    pub fn set_cell(
        self: Pin<&mut Self>,
        client: Pin<&mut ffi::SmushClient>,
        group: &String,
        index: usize,
        column: i32,
        data: &QVariant,
    ) -> i32 {
        let client = &mut client.rust_mut().client;
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
