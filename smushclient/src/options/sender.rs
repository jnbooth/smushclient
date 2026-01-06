use smushclient_plugins::Sender;

use super::decode::DecodeOption;
use super::encode::{EncodeOption, OptionValue};
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;

impl Optionable for Sender {
    fn get_option(&self, name: &LuaStr) -> OptionValue<'_> {
        match name {
            b"enabled" => self.enabled.encode(),
            b"group" => self.group.encode(),
            b"name" => self.label.encode(),
            b"omit_from_log" => self.omit_from_log.encode(),
            b"omit_from_output" => self.omit_from_output.encode(),
            b"one_shot" => self.one_shot.encode(),
            b"script" => self.script.encode(),
            b"send" => self.text.encode(),
            b"send_to" => self.send_to.encode(),
            b"user" => self.userdata.encode(),
            b"variable" => self.variable.encode(),
            _ => OptionValue::Null,
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"enabled" => self.enabled = value.decode()?,
            b"group" => self.group = value.decode()?,
            b"name" => self.label = value.decode()?,
            b"omit_from_output" => self.omit_from_output = value.decode()?,
            b"one_shot" => self.one_shot = value.decode()?,
            b"script" => self.script = value.decode()?,
            b"send" => self.text = value.decode()?,
            b"send_to" => self.send_to = value.decode()?,
            b"user" => self.userdata = value.decode()?,
            b"variable" => self.variable = value.decode()?,
            _ => return Err(OptionError::UnknownOption),
        }
        Ok(())
    }
}
