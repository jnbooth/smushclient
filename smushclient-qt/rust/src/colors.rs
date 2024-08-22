use cxx_qt_lib::{QColor, QString};

pub struct ColorEntryGeneric<TColor, TString> {
    pub ansi_foreground: TColor,
    pub custom_name: TString,
    pub custom_foreground: TColor,
    pub custom_background: TColor,
}

impl<TColor, TString> ColorEntryGeneric<TColor, TString> {
    pub const fn new(ansi: TColor, name: TString, foreground: TColor, background: TColor) -> Self {
        Self {
            ansi_foreground: ansi,
            custom_name: name,
            custom_foreground: foreground,
            custom_background: background,
        }
    }
}

#[derive(Debug, Default)]
pub struct Colors {
    // ANSI Color
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

    // Custom Color
    pub custom_names_0: QString,
    pub custom_names_1: QString,
    pub custom_names_2: QString,
    pub custom_names_3: QString,
    pub custom_names_4: QString,
    pub custom_names_5: QString,
    pub custom_names_6: QString,
    pub custom_names_7: QString,
    pub custom_names_8: QString,
    pub custom_names_9: QString,
    pub custom_names_10: QString,
    pub custom_names_11: QString,
    pub custom_names_12: QString,
    pub custom_names_13: QString,
    pub custom_names_14: QString,
    pub custom_names_15: QString,

    pub custom_colors_foreground_0: QColor,
    pub custom_colors_foreground_1: QColor,
    pub custom_colors_foreground_2: QColor,
    pub custom_colors_foreground_3: QColor,
    pub custom_colors_foreground_4: QColor,
    pub custom_colors_foreground_5: QColor,
    pub custom_colors_foreground_6: QColor,
    pub custom_colors_foreground_7: QColor,
    pub custom_colors_foreground_8: QColor,
    pub custom_colors_foreground_9: QColor,
    pub custom_colors_foreground_10: QColor,
    pub custom_colors_foreground_11: QColor,
    pub custom_colors_foreground_12: QColor,
    pub custom_colors_foreground_13: QColor,
    pub custom_colors_foreground_14: QColor,
    pub custom_colors_foreground_15: QColor,

    pub custom_colors_background_0: QColor,
    pub custom_colors_background_1: QColor,
    pub custom_colors_background_2: QColor,
    pub custom_colors_background_3: QColor,
    pub custom_colors_background_4: QColor,
    pub custom_colors_background_5: QColor,
    pub custom_colors_background_6: QColor,
    pub custom_colors_background_7: QColor,
    pub custom_colors_background_8: QColor,
    pub custom_colors_background_9: QColor,
    pub custom_colors_background_10: QColor,
    pub custom_colors_background_11: QColor,
    pub custom_colors_background_12: QColor,
    pub custom_colors_background_13: QColor,
    pub custom_colors_background_14: QColor,
    pub custom_colors_background_15: QColor,
}

type ColorEntry<'a> = ColorEntryGeneric<&'a QColor, &'a QString>;
type ColorEntryMut<'a> = ColorEntryGeneric<&'a mut QColor, &'a mut QString>;

impl Colors {
    pub fn array(&self) -> [ColorEntry; 16] {
        [
            ColorEntry {
                ansi_foreground: &self.ansi_colors_0,
                custom_name: &self.custom_names_0,
                custom_foreground: &self.custom_colors_foreground_0,
                custom_background: &self.custom_colors_background_0,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_1,
                custom_name: &self.custom_names_1,
                custom_foreground: &self.custom_colors_foreground_1,
                custom_background: &self.custom_colors_background_1,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_2,
                custom_name: &self.custom_names_2,
                custom_foreground: &self.custom_colors_foreground_2,
                custom_background: &self.custom_colors_background_2,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_3,
                custom_name: &self.custom_names_3,
                custom_foreground: &self.custom_colors_foreground_3,
                custom_background: &self.custom_colors_background_3,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_4,
                custom_name: &self.custom_names_4,
                custom_foreground: &self.custom_colors_foreground_4,
                custom_background: &self.custom_colors_background_4,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_5,
                custom_name: &self.custom_names_5,
                custom_foreground: &self.custom_colors_foreground_5,
                custom_background: &self.custom_colors_background_5,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_6,
                custom_name: &self.custom_names_6,
                custom_foreground: &self.custom_colors_foreground_6,
                custom_background: &self.custom_colors_background_6,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_7,
                custom_name: &self.custom_names_7,
                custom_foreground: &self.custom_colors_foreground_7,
                custom_background: &self.custom_colors_background_7,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_8,
                custom_name: &self.custom_names_8,
                custom_foreground: &self.custom_colors_foreground_8,
                custom_background: &self.custom_colors_background_8,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_9,
                custom_name: &self.custom_names_9,
                custom_foreground: &self.custom_colors_foreground_9,
                custom_background: &self.custom_colors_background_9,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_10,
                custom_name: &self.custom_names_10,
                custom_foreground: &self.custom_colors_foreground_10,
                custom_background: &self.custom_colors_background_10,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_11,
                custom_name: &self.custom_names_11,
                custom_foreground: &self.custom_colors_foreground_11,
                custom_background: &self.custom_colors_background_11,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_12,
                custom_name: &self.custom_names_12,
                custom_foreground: &self.custom_colors_foreground_12,
                custom_background: &self.custom_colors_background_12,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_13,
                custom_name: &self.custom_names_13,
                custom_foreground: &self.custom_colors_foreground_13,
                custom_background: &self.custom_colors_background_13,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_14,
                custom_name: &self.custom_names_14,
                custom_foreground: &self.custom_colors_foreground_14,
                custom_background: &self.custom_colors_background_14,
            },
            ColorEntry {
                ansi_foreground: &self.ansi_colors_15,
                custom_name: &self.custom_names_15,
                custom_foreground: &self.custom_colors_foreground_15,
                custom_background: &self.custom_colors_background_15,
            },
        ]
    }

    pub fn array_mut(&mut self) -> [ColorEntryMut; 16] {
        [
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_0,
                custom_name: &mut self.custom_names_0,
                custom_foreground: &mut self.custom_colors_foreground_0,
                custom_background: &mut self.custom_colors_background_0,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_1,
                custom_name: &mut self.custom_names_1,
                custom_foreground: &mut self.custom_colors_foreground_1,
                custom_background: &mut self.custom_colors_background_1,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_2,
                custom_name: &mut self.custom_names_2,
                custom_foreground: &mut self.custom_colors_foreground_2,
                custom_background: &mut self.custom_colors_background_2,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_3,
                custom_name: &mut self.custom_names_3,
                custom_foreground: &mut self.custom_colors_foreground_3,
                custom_background: &mut self.custom_colors_background_3,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_4,
                custom_name: &mut self.custom_names_4,
                custom_foreground: &mut self.custom_colors_foreground_4,
                custom_background: &mut self.custom_colors_background_4,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_5,
                custom_name: &mut self.custom_names_5,
                custom_foreground: &mut self.custom_colors_foreground_5,
                custom_background: &mut self.custom_colors_background_5,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_6,
                custom_name: &mut self.custom_names_6,
                custom_foreground: &mut self.custom_colors_foreground_6,
                custom_background: &mut self.custom_colors_background_6,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_7,
                custom_name: &mut self.custom_names_7,
                custom_foreground: &mut self.custom_colors_foreground_7,
                custom_background: &mut self.custom_colors_background_7,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_8,
                custom_name: &mut self.custom_names_8,
                custom_foreground: &mut self.custom_colors_foreground_8,
                custom_background: &mut self.custom_colors_background_8,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_9,
                custom_name: &mut self.custom_names_9,
                custom_foreground: &mut self.custom_colors_foreground_9,
                custom_background: &mut self.custom_colors_background_9,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_10,
                custom_name: &mut self.custom_names_10,
                custom_foreground: &mut self.custom_colors_foreground_10,
                custom_background: &mut self.custom_colors_background_10,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_11,
                custom_name: &mut self.custom_names_11,
                custom_foreground: &mut self.custom_colors_foreground_11,
                custom_background: &mut self.custom_colors_background_11,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_12,
                custom_name: &mut self.custom_names_12,
                custom_foreground: &mut self.custom_colors_foreground_12,
                custom_background: &mut self.custom_colors_background_12,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_13,
                custom_name: &mut self.custom_names_13,
                custom_foreground: &mut self.custom_colors_foreground_13,
                custom_background: &mut self.custom_colors_background_13,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_14,
                custom_name: &mut self.custom_names_14,
                custom_foreground: &mut self.custom_colors_foreground_14,
                custom_background: &mut self.custom_colors_background_14,
            },
            ColorEntryMut {
                ansi_foreground: &mut self.ansi_colors_15,
                custom_name: &mut self.custom_names_15,
                custom_foreground: &mut self.custom_colors_foreground_15,
                custom_background: &mut self.custom_colors_background_15,
            },
        ]
    }
}
