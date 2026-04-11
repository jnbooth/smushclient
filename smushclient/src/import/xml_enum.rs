use std::fmt;

use mud_transformer::UseMxp;
use serde::de::{self, Deserialize, Deserializer, Expected, Unexpected};

use crate::world::{AutoConnect, MxpDebugLevel, ScriptRecompile};

struct ExpectedRange(usize);

impl Expected for ExpectedRange {
    fn fmt(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        write!(formatter, "an integer between 0 and {}", self.0)
    }
}

pub(super) trait XmlEnum: Copy + Sized + 'static {
    const VARIANTS: &[Self];

    fn from_variant<E: de::Error>(v: i64) -> Result<Self, E> {
        #[cold]
        fn create_error<E: de::Error>(v: i64, len: usize) -> E {
            E::invalid_value(Unexpected::Signed(v), &ExpectedRange(len - 1))
        }

        let variant = usize::try_from(v).map_err(|_| create_error(v, Self::VARIANTS.len()))?;
        match Self::VARIANTS.get(variant) {
            Some(value) => Ok(*value),
            None => Err(create_error(v, Self::VARIANTS.len())),
        }
    }

    fn deserialize<'de, D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        let v = i64::deserialize(deserializer)?;
        Self::from_variant(v)
    }

    fn deserialize_opt<'de, D: Deserializer<'de>>(
        deserializer: D,
    ) -> Result<Option<Self>, D::Error> {
        let Some(v) = Option::deserialize(deserializer)? else {
            return Ok(None);
        };
        Ok(Some(Self::from_variant(v)?))
    }
}

impl XmlEnum for AutoConnect {
    const VARIANTS: &[Self] = &[Self::None, Self::Mush, Self::Diku, Self::Mxp];
}

impl XmlEnum for UseMxp {
    const VARIANTS: &[Self] = &[Self::Command, Self::Query, Self::Always, Self::Never];
}

impl XmlEnum for MxpDebugLevel {
    const VARIANTS: &[Self] = &[
        Self::None,
        Self::Error,
        Self::Warning,
        Self::Info,
        Self::All,
    ];
}

impl XmlEnum for ScriptRecompile {
    const VARIANTS: &[Self] = &[Self::Confirm, Self::Always, Self::Never];
}
