use std::collections::HashMap;
use std::ops::Index;

use smushclient_plugins::Sender;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct SenderMap {
    #[allow(clippy::vec_box)] // need static addresses
    group_names: Vec<Box<String>>,
    group_indices: HashMap<String, usize>,
    sender_indices: HashMap<String, Vec<usize>>,
}

impl SenderMap {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn is_empty(&self) -> bool {
        self.group_names.is_empty()
    }

    pub fn len(&self) -> usize {
        self.group_names.len()
    }

    pub fn group_len(&self, group_index: usize) -> usize {
        let Some(group_name) = self.group_names.get(group_index) else {
            return 0;
        };
        match self.sender_indices.get(&**group_name) {
            Some(indices) => indices.len(),
            None => 0,
        }
    }

    pub fn group_name(&self, group_index: usize) -> Option<&String> {
        Some(self.group_names.get(group_index)?)
    }

    pub fn group_index(&self, group: &str) -> Option<usize> {
        self.group_indices.get(group).copied()
    }

    pub fn sender_index(&self, group: &str, index: usize) -> Option<usize> {
        Some(*self.sender_indices.get(group)?.get(index)?)
    }

    pub fn recalculate<I>(&mut self, iter: I)
    where
        I: IntoIterator,
        I::Item: AsRef<Sender>,
    {
        for indices in self.sender_indices.values_mut() {
            indices.clear();
        }
        for (i, sender) in iter.into_iter().enumerate() {
            let group = &sender.as_ref().group;
            if let Some(present) = self.sender_indices.get_mut(group) {
                present.push(i);
            } else {
                self.sender_indices.insert(group.clone(), vec![i]);
            }
        }
        self.group_names.retain(|group| {
            self.sender_indices
                .get(group.as_str())
                .is_some_and(|indices| !indices.is_empty())
        });
        self.sender_indices.retain(|group, indices| {
            if indices.is_empty() {
                return false;
            }
            if let Err(i) = self
                .group_names
                .binary_search_by(|groupbox| groupbox.as_ref().cmp(group))
            {
                self.group_names.insert(i, Box::new(group.clone()));
            }
            true
        });
        for (i, group_name) in self.group_names.iter().enumerate() {
            let group_name: &String = group_name;
            if let Some(entry) = self.group_indices.get_mut(group_name) {
                *entry = i;
            } else {
                self.group_indices.insert(group_name.clone(), i);
            }
        }
    }
}

impl Index<&str> for SenderMap {
    type Output = [usize];

    fn index(&self, index: &str) -> &Self::Output {
        match self.sender_indices.get(index) {
            Some(indices) => indices.as_slice(),
            None => &[],
        }
    }
}

impl<T: AsRef<Sender>> FromIterator<T> for SenderMap {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> Self {
        let mut map = SenderMap::new();
        map.recalculate(iter);
        map
    }
}

impl<I> From<I> for SenderMap
where
    I: IntoIterator,
    I::Item: AsRef<Sender>,
{
    fn from(value: I) -> Self {
        Self::from_iter(value)
    }
}
