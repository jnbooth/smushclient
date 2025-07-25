use cxx_qt_lib::QColor;
use mud_transformer::mxp::RgbColor;

use crate::convert::Convert;

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct QColorPair {
    pub foreground: QColor,
    pub background: QColor,
}

#[allow(clippy::struct_field_names)]
#[derive(Debug, Default)]
pub struct Colors {
    pub ansi_0: QColor,
    pub ansi_1: QColor,
    pub ansi_2: QColor,
    pub ansi_3: QColor,
    pub ansi_4: QColor,
    pub ansi_5: QColor,
    pub ansi_6: QColor,
    pub ansi_7: QColor,
    pub ansi_8: QColor,
    pub ansi_9: QColor,
    pub ansi_10: QColor,
    pub ansi_11: QColor,
    pub ansi_12: QColor,
    pub ansi_13: QColor,
    pub ansi_14: QColor,
    pub ansi_15: QColor,
}

impl From<&[RgbColor; 16]> for Colors {
    fn from(value: &[RgbColor; 16]) -> Self {
        #[rustfmt::skip]
        let &[c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15] = value;
        Self {
            ansi_0: c0.convert(),
            ansi_1: c1.convert(),
            ansi_2: c2.convert(),
            ansi_3: c3.convert(),
            ansi_4: c4.convert(),
            ansi_5: c5.convert(),
            ansi_6: c6.convert(),
            ansi_7: c7.convert(),
            ansi_8: c8.convert(),
            ansi_9: c9.convert(),
            ansi_10: c10.convert(),
            ansi_11: c11.convert(),
            ansi_12: c12.convert(),
            ansi_13: c13.convert(),
            ansi_14: c14.convert(),
            ansi_15: c15.convert(),
        }
    }
}

impl From<&Colors> for [RgbColor; 16] {
    fn from(value: &Colors) -> Self {
        [
            value.ansi_0.convert(),
            value.ansi_1.convert(),
            value.ansi_2.convert(),
            value.ansi_3.convert(),
            value.ansi_4.convert(),
            value.ansi_5.convert(),
            value.ansi_6.convert(),
            value.ansi_7.convert(),
            value.ansi_8.convert(),
            value.ansi_9.convert(),
            value.ansi_10.convert(),
            value.ansi_11.convert(),
            value.ansi_12.convert(),
            value.ansi_13.convert(),
            value.ansi_14.convert(),
            value.ansi_15.convert(),
        ]
    }
}
