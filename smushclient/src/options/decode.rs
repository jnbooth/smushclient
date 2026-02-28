use mud_transformer::mxp::RgbColor;
use smushclient_plugins::SendTarget;

use super::color::DecodeColor;
use super::error::OptionError;
use crate::LuaStr;

pub trait FromOption: Sized {
    fn from_option(option: &LuaStr) -> Result<Self, OptionError>;
}

pub trait DecodeOption {
    fn decode<T: FromOption>(&self) -> Result<T, OptionError>;
}

impl DecodeOption for LuaStr {
    fn decode<T: FromOption>(&self) -> Result<T, OptionError> {
        T::from_option(self)
    }
}

impl FromOption for bool {
    fn from_option(option: &LuaStr) -> Result<Self, OptionError> {
        match option {
            b"y" | b"Y" | b"1" => Ok(true),
            b"n" | b"N" | b"0" => Ok(false),
            _ => Err(OptionError::OptionOutOfRange),
        }
    }
}

impl FromOption for Option<RgbColor> {
    fn from_option(option: &LuaStr) -> Result<Self, OptionError> {
        let name = str::from_utf8(option)?;
        let Some(hex) = name.strip_prefix("#") else {
            let color = RgbColor::named(name).ok_or(OptionError::OptionOutOfRange)?;
            return Ok(Some(color));
        };
        Ok(hex.parse::<i64>()?.decode_color()?)
    }
}

impl FromOption for String {
    fn from_option(option: &LuaStr) -> Result<Self, OptionError> {
        Ok(str::from_utf8(option)?.to_owned())
    }
}

impl FromOption for SendTarget {
    fn from_option(option: &LuaStr) -> Result<Self, OptionError> {
        match option {
            b"0" => Ok(SendTarget::World),
            b"1" => Ok(SendTarget::Command),
            b"2" => Ok(SendTarget::Output),
            b"3" => Ok(SendTarget::Status),
            b"4" => Ok(SendTarget::NotepadNew),
            b"5" => Ok(SendTarget::NotepadAppend),
            b"6" => Ok(SendTarget::Log),
            b"7" => Ok(SendTarget::NotepadReplace),
            b"8" => Ok(SendTarget::WorldDelay),
            b"9" => Ok(SendTarget::Variable),
            b"10" => Ok(SendTarget::Execute),
            b"11" => Ok(SendTarget::Speedwalk),
            b"12" => Ok(SendTarget::Script),
            b"13" => Ok(SendTarget::WorldImmediate),
            b"14" => Ok(SendTarget::ScriptAfterOmit),
            _ => Err(OptionError::OptionOutOfRange),
        }
    }
}

macro_rules! impl_parse {
    ($t:ty) => {
        impl FromOption for $t {
            fn from_option(option: &LuaStr) -> Result<Self, OptionError> {
                Ok(str::from_utf8(option)?.parse()?)
            }
        }
    };
}

impl_parse!(u8);
impl_parse!(i16);
impl_parse!(i32);
impl_parse!(i64);
