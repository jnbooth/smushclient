use std::iter;

use serde::de::SeqAccess;
use serde::ser::{SerializeStruct, Serializer};
use serde::{Deserialize, Serialize};

use crate::cursor_vec::CursorVec;
use crate::error::ImportError;

pub trait XmlIterable: Sized + 'static {
    const TAG: &'static str;

    type Xml<'a>: From<&'a Self>
        + TryInto<Self, Error: Into<ImportError>>
        + Deserialize<'a>
        + Serialize;

    fn from_xml_str<T>(s: &str) -> Result<T, ImportError>
    where
        T: FromIterator<Self>,
    {
        let mut deserializer = &mut quick_xml::de::Deserializer::from_str(s);
        iter::from_fn(move || match deserializer.next_element::<Self::Xml<'_>>() {
            Err(e) => Some(Err(e.into())),
            Ok(None) => None,
            Ok(Some(value)) => Some(value.try_into().map_err(Into::into)),
        })
        .collect()
    }

    fn to_xml_string<'a, I>(iter: I) -> Result<String, quick_xml::SeError>
    where
        I: IntoIterator<Item = &'a Self>,
        I::IntoIter: Clone,
    {
        struct ListIter<I>(I);
        impl<I> Serialize for ListIter<I>
        where
            I: Clone + Iterator,
            I::Item: Serialize,
        {
            fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
                serializer.collect_seq(self.0.clone())
            }
        }

        let mut buf = String::new();
        let serializer = quick_xml::se::Serializer::new(&mut buf);
        let mut state = serializer.serialize_struct("", 1)?;
        state.serialize_field(Self::TAG, &ListIter(iter.into_iter().map(Self::Xml::from)))?;
        state.end()?;
        buf.truncate(buf.len() - const { "</>".len() });
        buf.drain(..const { "<>".len() });
        Ok(buf)
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

#[inline]
#[allow(clippy::trivially_copy_pass_by_ref)]
pub fn is_true(value: &bool) -> bool {
    *value
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
#[serde(default)]
pub struct XmlVec<T> {
    #[serde(rename = "@muclient_version")]
    pub muclient_version: Option<String>,
    #[serde(rename = "@world_file_version")]
    pub world_file_version: Option<u32>,
    #[serde(rename = "@date_saved")]
    pub date_saved: Option<String>,
    #[serde(rename = "$value")]
    pub elements: Vec<T>,
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

impl<T, U: Ord> TryFrom<XmlVec<T>> for CursorVec<U>
where
    U: TryFrom<T>,
{
    type Error = <U as TryFrom<T>>::Error;

    fn try_from(value: XmlVec<T>) -> Result<Self, Self::Error> {
        let mut items: Vec<U> = value
            .elements
            .into_iter()
            .map(U::try_from)
            .collect::<Result<_, _>>()?;
        items.sort_unstable();
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
        let to_xml = XmlIterable::to_xml_string(&triggers).expect("error serializing triggers");
        let mut from_xml: Vec<Trigger> =
            XmlIterable::from_xml_str(&to_xml).expect("error deserializing triggers");
        from_xml[0].id = triggers[0].id;
        from_xml[1].id = triggers[1].id;
        assert_eq!(from_xml, triggers);
    }
}
