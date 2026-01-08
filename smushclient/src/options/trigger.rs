use smushclient_plugins::Trigger;

use super::decode::DecodeOption;
use super::encode::{EncodeOption, OptionValue};
use super::error::OptionError;
use super::optionable::Optionable;
use crate::LuaStr;

impl Optionable for Trigger {
    fn get_option(&self, name: &LuaStr) -> OptionValue<'_> {
        match name {
            b"clipboard_arg" => self.clipboard_arg.encode(),
            b"italic" => self.make_italic.encode(),
            b"lines_to_match" => self.lines_to_match.encode(),
            b"lowercase_wildcard" => self.lowercase_wildcard.encode(),
            b"multi_line" => self.multi_line.encode(),
            b"new_style" => {
                let mut changes: i32 = 0;
                if self.make_bold {
                    changes |= 1;
                }
                if self.make_underline {
                    changes |= 2;
                }
                if self.make_italic {
                    changes |= 4;
                }
                changes.encode()
            }
            b"other_back_colour" => self.background_color.encode(),
            b"other_text_colour" => self.foreground_color.encode(),
            b"sound" => self.sound.encode(),
            b"sound_if_inactive" => self.sound_if_inactive.encode(),
            _ => self.reaction.get_option(name),
        }
    }

    fn set_option(&mut self, name: &LuaStr, value: &LuaStr) -> Result<(), OptionError> {
        match name {
            b"clipboard_arg" => self.clipboard_arg = value.decode()?,
            b"italic" => self.make_italic = value.decode()?,
            b"lines_to_match" => self.lines_to_match = value.decode()?,
            b"lowercase_wildcard" => self.lowercase_wildcard = value.decode()?,
            b"match" => {
                let pattern: String = value.decode()?;
                if pattern.is_empty() {
                    return Err(OptionError::TriggerCannotBeEmpty);
                }
                self.set_pattern(pattern)?;
            }
            b"multi_line" => self.multi_line = value.decode()?,
            b"new_style" => {
                let flags: u8 = value.decode()?;
                self.make_bold = (flags & 1) != 0;
                self.make_underline = (flags & 2) != 0;
                self.make_italic = (flags & 4) != 0;
            }
            b"other_back_colour" => self.background_color = value.decode()?,
            b"other_text_colour" => self.foreground_color = value.decode()?,
            b"sound" => self.sound = value.decode()?,
            b"sound_if_inactive" => self.sound_if_inactive = value.decode()?,
            _ => self.reaction.set_option(name, value)?,
        }
        Ok(())
    }
}
