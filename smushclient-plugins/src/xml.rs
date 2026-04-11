use std::io::BufRead;
use std::marker::PhantomData;
use std::ops::{Deref, DerefMut};
use std::{fmt, vec};

use quick_xml::de::{Deserializer, IoReader, SliceReader, XmlRead};
use quick_xml::se::{EmptyElementHandling, SeError, Serializer, WriteResult};
pub use quick_xml::{DeError as XmlError, SeError as XmlSerError};
use serde::de::SeqAccess;
use serde::ser::Serializer as _;
use serde::{Deserialize, Serialize};

use crate::CursorVec;
use crate::error::ImportError;
pub use crate::send::{XmlAlias, XmlTimer, XmlTrigger};

pub trait XmlIterable: Sized + 'static {
    type Xml<'a>: From<&'a Self>
        + TryInto<Self, Error: Into<ImportError>>
        + Deserialize<'a>
        + Serialize;

    const TAG: &'static str;

    fn from_xml_list<R>(reader: R) -> XmlIter<'static, Self, IoReader<R>>
    where
        R: BufRead,
    {
        Deserializer::from_reader(reader).into()
    }

    fn from_xml_list_str(s: &str) -> XmlIter<'_, Self> {
        Deserializer::from_str(s).into()
    }

    fn from_xml<R>(reader: R) -> Result<Self, ImportError>
    where
        R: BufRead,
    {
        Self::Xml::<'_>::deserialize(&mut Deserializer::from_reader(reader))?
            .try_into()
            .map_err(Into::into)
    }

    fn from_xml_str(s: &str) -> Result<Self, ImportError> {
        Self::Xml::<'_>::deserialize(&mut Deserializer::from_str(s))?
            .try_into()
            .map_err(Into::into)
    }

    fn to_xml_string(&self) -> Result<String, SeError> {
        let mut buf = String::new();
        self.write_xml(&mut buf)?;
        Ok(buf)
    }

    fn to_xml_list_string<'a, I>(iter: I) -> Result<String, SeError>
    where
        I: IntoIterator<Item = &'a Self>,
    {
        let mut buf = String::new();
        Self::write_xml_list(&mut buf, iter)?;
        Ok(buf)
    }

    fn write_xml<W>(&self, mut writer: W) -> Result<WriteResult, SeError>
    where
        W: fmt::Write,
    {
        let mut serializer = Serializer::new(&mut writer);
        serializer.empty_element_handling(EmptyElementHandling::Expanded);
        Self::Xml::from(self).serialize(serializer)
    }

    fn write_xml_list<'a, I, W>(mut writer: W, iter: I) -> Result<WriteResult, SeError>
    where
        I: IntoIterator<Item = &'a Self>,
        W: fmt::Write,
    {
        let mut serializer = Serializer::with_root(&mut writer, Some(Self::TAG)).unwrap();
        serializer.empty_element_handling(EmptyElementHandling::Expanded);
        serializer.collect_seq(iter.into_iter().map(Self::Xml::from))
    }
}

pub struct XmlIter<'a, T, D = SliceReader<'a>>
where
    D: XmlRead<'a>,
{
    inner: Deserializer<'a, D>,
    marker: PhantomData<T>,
}

impl<'a, T, D> From<Deserializer<'a, D>> for XmlIter<'a, T, D>
where
    D: XmlRead<'a>,
{
    fn from(value: Deserializer<'a, D>) -> Self {
        Self {
            inner: value,
            marker: PhantomData,
        }
    }
}

impl<'a, T, D> Iterator for XmlIter<'a, T, D>
where
    D: XmlRead<'a>,
    T: XmlIterable,
{
    type Item = Result<T, ImportError>;

    #[inline]
    fn next(&mut self) -> Option<Self::Item> {
        match (&mut self.inner).next_element::<T::Xml<'_>>() {
            Err(e) => Some(Err(e.into())),
            Ok(None) => None,
            Ok(Some(value)) => Some(value.try_into().map_err(Into::into)),
        }
    }
}

pub mod bool_serde {
    use serde::{Serialize, Serializer};
    pub use serde_this_or_that::as_bool as deserialize;

    #[allow(clippy::trivially_copy_pass_by_ref)]
    pub fn serialize<S: Serializer>(value: &bool, serializer: S) -> Result<S::Ok, S::Error> {
        (if *value { "y" } else { "n" }).serialize(serializer)
    }
}

#[inline]
pub fn is_default<T: Default + PartialEq>(value: &T) -> bool {
    value == &T::default()
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
#[serde(default)]
pub struct XmlVec<T> {
    #[serde(rename = "$value")]
    pub elements: Vec<T>,
    #[serde(rename = "@muclient_version")]
    pub muclient_version: Option<String>,
    #[serde(rename = "@world_file_version")]
    pub world_file_version: Option<u32>,
    #[serde(rename = "@date_saved")]
    pub date_saved: Option<String>,
}

impl<T> Deref for XmlVec<T> {
    type Target = Vec<T>;

    fn deref(&self) -> &Self::Target {
        &self.elements
    }
}

impl<T> DerefMut for XmlVec<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.elements
    }
}

impl<T> Default for XmlVec<T> {
    fn default() -> Self {
        Self {
            muclient_version: None,
            world_file_version: None,
            date_saved: None,
            elements: Vec::new(),
        }
    }
}

impl<T> XmlVec<T> {
    pub const fn is_empty(&self) -> bool {
        self.elements.is_empty()
    }
}

impl<T> IntoIterator for XmlVec<T> {
    type Item = T;

    type IntoIter = vec::IntoIter<T>;

    #[inline]
    fn into_iter(self) -> Self::IntoIter {
        self.elements.into_iter()
    }
}

impl<'a, T, U> From<&'a [T]> for XmlVec<U>
where
    T: Ord,
    U: From<&'a T>,
{
    fn from(value: &'a [T]) -> Self {
        Self {
            elements: value.iter().map(U::from).collect(),
            ..Default::default()
        }
    }
}

impl<T, U> TryFrom<XmlVec<T>> for Vec<U>
where
    U: TryFrom<T> + Ord,
{
    type Error = <U as TryFrom<T>>::Error;

    fn try_from(value: XmlVec<T>) -> Result<Self, Self::Error> {
        let mut items: Vec<U> = value
            .elements
            .into_iter()
            .map(U::try_from)
            .collect::<Result<_, _>>()?;
        items.sort_unstable();
        Ok(items)
    }
}

impl<T, U> TryFrom<XmlVec<T>> for CursorVec<U>
where
    U: TryFrom<T> + Ord,
{
    type Error = <U as TryFrom<T>>::Error;

    fn try_from(value: XmlVec<T>) -> Result<Self, Self::Error> {
        let items: Vec<U> = value.try_into()?;
        Ok(items.into())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::send::Trigger;

    #[test]
    fn xml_roundtrip() {
        let triggers = vec![Trigger::default(), Trigger::default()];
        let to_xml =
            XmlIterable::to_xml_list_string(&triggers).expect("error serializing triggers");
        assert_eq!(
            to_xml,
            r#"<trigger enabled="y" sequence="100"></trigger><trigger enabled="y" sequence="100"></trigger>"#
        );
        let mut from_xml: Vec<Trigger> = XmlIterable::from_xml_list_str(&to_xml)
            .collect::<Result<_, _>>()
            .expect("error deserializing triggers");
        from_xml[0].id = triggers[0].id;
        from_xml[1].id = triggers[1].id;
        assert_eq!(from_xml, triggers);
    }
}
