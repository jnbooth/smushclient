use smushclient_plugins::Alias;

use super::decode::DecodeOption;
use super::encode::{EncodeOption, OptionValue};
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;

impl Optionable for Alias {
    fn get_option(&self, name: &LuaStr) -> OptionValue<'_> {
        match name {
            b"echo_alias" => self.echo_alias.encode(),
            b"menu" => self.menu.encode(),
            b"omit_from_command_history" => self.omit_from_command_history.encode(),
            _ => self.reaction.get_option(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"echo_alias" => self.echo_alias = value.decode()?,
            b"match" => {
                let pattern: String = value.decode()?;
                if pattern.is_empty() {
                    return Err(OptionError::AliasCannotBeEmpty);
                }
                self.set_pattern(pattern)?;
            }
            b"menu" => self.menu = value.decode()?,
            b"omit_from_command_history" => self.omit_from_command_history = value.decode()?,
            _ => self.reaction.set_option(name, value)?,
        }
        Ok(())
    }
}
