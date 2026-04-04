use smushclient_plugins::Reaction;

use super::decode::DecodeOption;
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;
use crate::get_info::InfoVisitor;

impl Optionable for Reaction {
    fn get_option<V: InfoVisitor>(&self, name: &LuaStr) -> V::Output {
        match name {
            b"expand_variables" => V::visit(self.expand_variables),
            b"ignore_case" => V::visit(self.ignore_case),
            b"keep_evaluating" => V::visit(self.keep_evaluating),
            b"match" => V::visit(&self.pattern),
            b"regexp" => V::visit(self.is_regex),
            b"repeat" => V::visit(self.repeats),
            b"sequence" => V::visit(self.sequence),
            _ => self.send.get_option::<V>(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"expand_variables" => self.expand_variables = value.decode()?,
            b"ignore_case" => self.set_ignore_case(value.decode()?)?,
            b"keep_evaluating" => self.keep_evaluating = value.decode()?,
            b"regexp" => self.set_is_regex(value.decode()?)?,
            b"repeat" => self.repeats = value.decode()?,
            b"sequence" => self.sequence = value.decode_in_range(..=10000)?,
            _ => self.send.set_option(name, value)?,
        }
        Ok(())
    }
}
