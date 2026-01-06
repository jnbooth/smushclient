use smushclient_plugins::Timer;

use super::decode::DecodeOption;
use super::encode::{EncodeOption, OptionValue};
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;

impl Optionable for Timer {
    fn get_option(&self, name: &LuaStr) -> OptionValue<'_> {
        match name {
            b"active_closed" => self.active_closed.encode(),
            _ => self.send.get_option(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"active_closed" => self.active_closed = value.decode()?,
            _ => self.send.set_option(name, value)?,
        }
        Ok(())
    }
}
