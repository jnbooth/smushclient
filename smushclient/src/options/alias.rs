use smushclient_plugins::Alias;

use super::decode::DecodeOption;
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;
use crate::get_info::InfoVisitor;

impl Optionable for Alias {
    fn get_option<V: InfoVisitor>(&self, name: &LuaStr) -> V::Output {
        match name {
            b"echo_alias" => V::visit(self.echo_alias),
            b"menu" => V::visit(self.menu),
            b"omit_from_command_history" => V::visit(self.omit_from_command_history),
            _ => self.reaction.get_option::<V>(name),
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
