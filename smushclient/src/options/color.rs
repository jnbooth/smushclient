use mud_transformer::mxp::RgbColor;

use super::error::SetOptionError;

const NO_COLOR: i64 = 0xFFFFFFFF;

pub(crate) trait EncodeColor: Sized {
    fn encode_color(self) -> i64;
}

impl EncodeColor for RgbColor {
    fn encode_color(self) -> i64 {
        (i64::from(self.b) << 16) | (i64::from(self.g) << 8) | i64::from(self.r)
    }
}

impl EncodeColor for Option<RgbColor> {
    fn encode_color(self) -> i64 {
        match self {
            Some(color) => color.encode_color(),
            None => NO_COLOR,
        }
    }
}

pub(crate) trait DecodeColor {
    fn decode_color(self) -> Result<Option<RgbColor>, SetOptionError>;
}

impl DecodeColor for i64 {
    fn decode_color(self) -> Result<Option<RgbColor>, SetOptionError> {
        if !(0..=0xFFFFFF).contains(&self) {
            return Err(SetOptionError::OptionOutOfRange);
        }
        #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
        Ok(Some(RgbColor {
            r: (self & 0xFF) as u8,
            g: ((self >> 8) & 0xFF) as u8,
            b: ((self >> 16) & 0xFF) as u8,
        }))
    }
}
