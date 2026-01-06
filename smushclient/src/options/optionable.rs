use super::encode::OptionValue;
use super::error::OptionError;
use crate::LuaStr;

pub trait Optionable {
    fn get_option(&self, name: &LuaStr) -> OptionValue<'_>;
    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError>;
}
