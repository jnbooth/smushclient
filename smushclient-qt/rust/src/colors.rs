use cxx_qt_lib::QColor;
use mud_transformer::mxp::RgbColor;

use crate::convert::Convert;

#[allow(clippy::struct_field_names)]
#[derive(Debug, Default)]
pub struct Colors {
    pub ansi_colors_0: QColor,
    pub ansi_colors_1: QColor,
    pub ansi_colors_2: QColor,
    pub ansi_colors_3: QColor,
    pub ansi_colors_4: QColor,
    pub ansi_colors_5: QColor,
    pub ansi_colors_6: QColor,
    pub ansi_colors_7: QColor,
    pub ansi_colors_8: QColor,
    pub ansi_colors_9: QColor,
    pub ansi_colors_10: QColor,
    pub ansi_colors_11: QColor,
    pub ansi_colors_12: QColor,
    pub ansi_colors_13: QColor,
    pub ansi_colors_14: QColor,
    pub ansi_colors_15: QColor,
}

impl From<&[RgbColor; 16]> for Colors {
    fn from(value: &[RgbColor; 16]) -> Self {
        let &[c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15] = value;
        Self {
            ansi_colors_0: c0.convert(),
            ansi_colors_1: c1.convert(),
            ansi_colors_2: c2.convert(),
            ansi_colors_3: c3.convert(),
            ansi_colors_4: c4.convert(),
            ansi_colors_5: c5.convert(),
            ansi_colors_6: c6.convert(),
            ansi_colors_7: c7.convert(),
            ansi_colors_8: c8.convert(),
            ansi_colors_9: c9.convert(),
            ansi_colors_10: c10.convert(),
            ansi_colors_11: c11.convert(),
            ansi_colors_12: c12.convert(),
            ansi_colors_13: c13.convert(),
            ansi_colors_14: c14.convert(),
            ansi_colors_15: c15.convert(),
        }
    }
}

impl From<&Colors> for [RgbColor; 16] {
    fn from(value: &Colors) -> Self {
        [
            value.ansi_colors_0.convert(),
            value.ansi_colors_1.convert(),
            value.ansi_colors_2.convert(),
            value.ansi_colors_3.convert(),
            value.ansi_colors_4.convert(),
            value.ansi_colors_5.convert(),
            value.ansi_colors_6.convert(),
            value.ansi_colors_7.convert(),
            value.ansi_colors_8.convert(),
            value.ansi_colors_9.convert(),
            value.ansi_colors_10.convert(),
            value.ansi_colors_11.convert(),
            value.ansi_colors_12.convert(),
            value.ansi_colors_13.convert(),
            value.ansi_colors_14.convert(),
            value.ansi_colors_15.convert(),
        ]
    }
}
