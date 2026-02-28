use std::cell::Ref;

use mud_transformer::mxp::RgbColor;
use smushclient_plugins::SendTarget;

#[derive(Debug, Default)]
pub enum OptionValue<'a> {
    #[default]
    Null,
    Alpha(&'a str),
    AlphaBorrow(Ref<'a, str>),
    AlphaOwned(String),
    Color(RgbColor),
    Numeric(i64),
}

pub trait EncodeOption<'a> {
    fn encode(self) -> OptionValue<'a>;
}

macro_rules! impl_numeric {
    ($t:ty) => {
        impl EncodeOption<'static> for $t {
            fn encode(self) -> OptionValue<'static> {
                #[allow(clippy::cast_lossless)]
                OptionValue::Numeric(self as _)
            }
        }
    };
}

impl_numeric!(bool);
impl_numeric!(u8);
impl_numeric!(i16);
impl_numeric!(i32);
impl_numeric!(i64);
impl_numeric!(SendTarget);

impl EncodeOption<'static> for RgbColor {
    fn encode(self) -> OptionValue<'static> {
        OptionValue::Color(self)
    }
}

impl<'a> EncodeOption<'a> for &'a str {
    fn encode(self) -> OptionValue<'a> {
        OptionValue::Alpha(self)
    }
}

impl<'a> EncodeOption<'a> for &'a String {
    fn encode(self) -> OptionValue<'a> {
        OptionValue::Alpha(self.as_str())
    }
}
