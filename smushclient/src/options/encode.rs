use std::num::NonZero;

use mud_transformer::mxp::RgbColor;
use smushclient_plugins::SendTarget;

use crate::LuaStr;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum OptionValue<'a> {
    #[default]
    Null,
    Alpha(&'a LuaStr),
    Color(RgbColor),
    Numeric(i32),
}

pub trait EncodeOption<'a> {
    fn encode(self) -> OptionValue<'a>;
}

macro_rules! impl_numeric {
    ($t:ty) => {
        impl EncodeOption<'static> for $t {
            fn encode(self) -> OptionValue<'static> {
                #[allow(clippy::cast_lossless)]
                OptionValue::Numeric(self as i32)
            }
        }
    };
}

impl_numeric!(bool);
impl_numeric!(u8);
impl_numeric!(i16);
impl_numeric!(i32);
impl_numeric!(SendTarget);

impl EncodeOption<'static> for RgbColor {
    fn encode(self) -> OptionValue<'static> {
        OptionValue::Color(self)
    }
}

impl<'a> EncodeOption<'a> for &'a LuaStr {
    fn encode(self) -> OptionValue<'a> {
        OptionValue::Alpha(self)
    }
}

impl<'a> EncodeOption<'a> for &'a str {
    fn encode(self) -> OptionValue<'a> {
        OptionValue::Alpha(self.as_bytes())
    }
}

impl<'a> EncodeOption<'a> for &'a String {
    fn encode(self) -> OptionValue<'a> {
        OptionValue::Alpha(self.as_bytes())
    }
}

impl EncodeOption<'static> for Option<NonZero<u8>> {
    fn encode(self) -> OptionValue<'static> {
        match self {
            Some(value) => OptionValue::Numeric(i32::from(value.get())),
            None => OptionValue::Numeric(0),
        }
    }
}
