use smushclient_plugins::Sender;

use super::decode::DecodeOption;
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;
use crate::get_info::InfoVisitor;

impl Optionable for Sender {
    fn get_option<V: InfoVisitor>(&self, name: &LuaStr) -> V::Output {
        match name {
            b"enabled" => V::visit(self.enabled),
            b"group" => V::visit(&self.group),
            b"name" => V::visit(&self.label),
            b"omit_from_log" => V::visit(self.omit_from_log),
            b"omit_from_output" => V::visit(self.omit_from_output),
            b"one_shot" => V::visit(self.one_shot),
            b"script" => V::visit(&self.script),
            b"send" => V::visit(&self.text),
            b"send_to" => V::visit(self.send_to),
            b"user" => V::visit(self.userdata),
            b"variable" => V::visit(&self.variable),
            _ => V::visit_none(),
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
