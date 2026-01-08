use std::num::NonZero;

use mud_transformer::mxp::RgbColor;
use smushclient_plugins::SendTarget;

use super::error::OptionError;
use crate::LuaStr;

pub trait FromOption: Sized {
    fn from_option(option: &LuaStr) -> Option<Self>;
}

pub trait DecodeOption {
    fn decode<T: FromOption>(&self) -> Result<T, OptionError>;
}

impl DecodeOption for LuaStr {
    fn decode<T: FromOption>(&self) -> Result<T, OptionError> {
        T::from_option(self).ok_or(OptionError::OptionOutOfRange)
    }
}

impl FromOption for bool {
    fn from_option(option: &LuaStr) -> Option<Self> {
        match option {
            b"y" | b"Y" | b"1" => Some(true),
            b"n" | b"N" | b"0" => Some(false),
            _ => None,
        }
    }
}

impl FromOption for RgbColor {
    fn from_option(option: &LuaStr) -> Option<Self> {
        let name = str::from_utf8(option).ok()?;
        RgbColor::named(name)
    }
}

impl FromOption for String {
    fn from_option(option: &LuaStr) -> Option<Self> {
        match str::from_utf8(option) {
            Ok(s) => Some(s.to_owned()),
            Err(_) => None,
        }
    }
}

impl FromOption for SendTarget {
    fn from_option(option: &LuaStr) -> Option<Self> {
        match option {
            b"0" => Some(SendTarget::World),
            b"1" => Some(SendTarget::Command),
            b"2" => Some(SendTarget::Output),
            b"3" => Some(SendTarget::Status),
            b"4" => Some(SendTarget::NotepadNew),
            b"5" => Some(SendTarget::NotepadAppend),
            b"6" => Some(SendTarget::Log),
            b"7" => Some(SendTarget::NotepadReplace),
            b"8" => Some(SendTarget::WorldDelay),
            b"9" => Some(SendTarget::Variable),
            b"10" => Some(SendTarget::Execute),
            b"11" => Some(SendTarget::Speedwalk),
            b"12" => Some(SendTarget::Script),
            b"13" => Some(SendTarget::WorldImmediate),
            b"14" => Some(SendTarget::ScriptAfterOmit),
            _ => None,
        }
    }
}

macro_rules! impl_parse {
    ($t:ty) => {
        impl FromOption for $t {
            fn from_option(option: &LuaStr) -> Option<Self> {
                str::from_utf8(option).ok()?.parse().ok()
            }
        }
    };
}

impl_parse!(u8);
impl_parse!(i16);
impl_parse!(i32);

impl FromOption for Option<NonZero<u8>> {
    fn from_option(option: &LuaStr) -> Option<Self> {
        u8::from_option(option).map(NonZero::new)
    }
}
