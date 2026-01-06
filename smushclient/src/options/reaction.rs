use smushclient_plugins::Reaction;

use super::decode::DecodeOption;
use super::encode::{EncodeOption, OptionValue};
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;

impl Optionable for Reaction {
    fn get_option(&self, name: &LuaStr) -> OptionValue<'_> {
        match name {
            b"expand_variables" => self.expand_variables.encode(),
            b"ignore_case" => self.ignore_case.encode(),
            b"keep_evaluating" => self.keep_evaluating.encode(),
            b"match" => self.pattern.encode(),
            b"regexp" => self.is_regex.encode(),
            b"repeat" => self.repeats.encode(),
            b"sequence" => self.sequence.encode(),
            _ => self.send.get_option(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"expand_variables" => self.expand_variables = value.decode()?,
            b"ignore_case" => self.ignore_case = value.decode()?,
            b"keep_evaluating" => self.keep_evaluating = value.decode()?,
            b"regexp" => self.set_is_regex(value.decode()?)?,
            b"repeat" => self.repeats = value.decode()?,
            b"sequence" => self.sequence = value.decode()?,
            _ => self.send.set_option(name, value)?,
        }
        Ok(())
    }
}
