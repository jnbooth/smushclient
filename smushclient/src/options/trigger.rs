use mud_transformer::opt::mxp::RgbColor;
use smushclient_plugins::Trigger;

use super::decode::DecodeOption;
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;
use crate::get_info::InfoVisitor;

impl Optionable for Trigger {
    fn get_option<V: InfoVisitor>(&self, name: &LuaStr) -> V::Output {
        match name {
            b"clipboard_arg" => V::visit(self.clipboard_arg),
            b"colour_change_type" => V::visit(self.color_change_byte()),
            b"lines_to_match" => V::visit(self.lines_to_match),
            b"lowercase_wildcard" => V::visit(self.lowercase_wildcard),
            b"multi_line" => V::visit(self.multi_line),
            b"new_style" => V::visit(self.style_byte()),
            b"other_back_colour" => V::visit(self.background_color),
            b"other_text_colour" => V::visit(self.foreground_color),
            b"sound" => V::visit(&self.sound),
            b"sound_if_inactive" => V::visit(self.sound_if_inactive),
            b"custom_colour" | b"match_style" => V::visit(0),
            b"inverse" | b"italic" => V::visit(false),
            _ => self.reaction.get_option::<V>(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"clipboard_arg" => self.clipboard_arg = value.decode()?,
            b"colour_change_type" => {
                let change = value.decode_in_range(..=2)?;
                self.change_background = change != 1;
                self.change_foreground = change != 2;
            }
            b"lines_to_match" => self.lines_to_match = value.decode_in_range(..=200)?,
            b"lowercase_wildcard" => self.lowercase_wildcard = value.decode()?,
            b"match" => {
                let pattern: String = value.decode()?;
                if pattern.is_empty() {
                    return Err(OptionError::TriggerCannotBeEmpty);
                }
                self.set_pattern(pattern)?;
            }
            b"multi_line" => self.multi_line = value.decode()?,
            b"new_style" => self.set_style_byte(value.decode_in_range(..8)?),
            b"other_back_colour" => {
                if let Some(color) = value.decode()? {
                    self.background_color = color;
                } else {
                    self.background_color = RgbColor::BLACK;
                    self.change_background = false;
                }
            }
            b"other_text_colour" => {
                if let Some(color) = value.decode()? {
                    self.foreground_color = color;
                } else {
                    self.foreground_color = RgbColor::WHITE;
                    self.change_foreground = false;
                }
            }
            b"sound" => self.sound = value.decode()?,
            b"sound_if_inactive" => self.sound_if_inactive = value.decode()?,
            _ => self.reaction.set_option(name, value)?,
        }
        Ok(())
    }
}
