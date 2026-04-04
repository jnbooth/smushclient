use super::error::OptionError;
use crate::LuaStr;
use crate::get_info::InfoVisitor;

pub trait Optionable {
    fn get_option<V: InfoVisitor>(&self, name: &LuaStr) -> V::Output;
    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError>;
}
