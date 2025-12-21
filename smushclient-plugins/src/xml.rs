use std::{fmt, iter};

use serde::de::{Deserialize, SeqAccess};
use serde::ser::{Serialize, SerializeStruct, Serializer};

pub trait XmlIterable: Sized + 'static {
    const TAG: &'static str;

    type Xml<'a>: From<&'a Self> + TryInto<Self, Error: fmt::Display> + Deserialize<'a> + Serialize;

    fn from_xml_str<T>(s: &str) -> Result<T, quick_xml::DeError>
    where
        T: FromIterator<Self>,
    {
        let mut deserializer = &mut quick_xml::de::Deserializer::from_str(s);
        iter::from_fn(move || match deserializer.next_element::<Self::Xml<'_>>() {
            Err(e) => Some(Err(e)),
            Ok(Some(value)) => Some(Ok(value)),
            Ok(None) => None,
        })
        .enumerate()
        .map(|(n, item)| {
            item?.try_into().map_err(|e| {
                quick_xml::DeError::Custom(format!("Invalid pattern in item {}: {e}", n + 1))
            })
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::send::Trigger;

    #[test]
    fn xml_roundtrip() {
        let triggers = vec![Trigger::default(), Trigger::default()];
        let to_xml = XmlIterable::to_xml_string(&triggers).expect("error serializing triggers");
        let from_xml: Vec<Trigger> =
            XmlIterable::from_xml_str(&to_xml).expect("error deserializing triggers");
        assert_eq!(from_xml, triggers);
    }
}
