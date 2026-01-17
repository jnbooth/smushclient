use std::num::NonZero;
use std::{slice, vec};

use mud_transformer::mxp::RgbColor;
use serde::Deserialize;
use smushclient_plugins::{Trigger, XmlIterable};

#[derive(Copy, Clone, Debug, Default)]
pub(super) struct ColorPair {
    pub text: RgbColor,
    pub back: RgbColor,
}

#[derive(Clone, Debug, Default)]
pub(super) struct CustomColours {
    pairs: Vec<Option<ColorPair>>,
}

impl CustomColours {
    pub fn get(&self, index: usize) -> Option<ColorPair> {
        let index = index.checked_sub(1)?;
        match self.pairs.get(index) {
            Some(Some(pair)) => Some(*pair),
            _ => None,
        }
    }

    pub fn apply_to_trigger(&self, trigger: &mut <Trigger as XmlIterable>::Xml<'_>) {
        let Some(pair) = self.get(trigger.custom_colour) else {
            return;
        };
        trigger.other_back_colour.get_or_insert(pair.text);
        trigger.other_text_colour.get_or_insert(pair.back);
    }
}

#[derive(Debug, Default, Deserialize)]
#[serde(from = "ColoursXml")]
pub(super) struct Colours {
    pub ansi: [RgbColor; 16],
    pub custom: CustomColours,
}

impl From<ColoursXml> for Colours {
    fn from(value: ColoursXml) -> Self {
        let mut ansi = *RgbColor::XTERM_16;
        for color in value.ansi.normal.into_iter().chain(value.ansi.bold) {
            if let Some(slot) = ansi.get_mut(color.seq.get() - 1) {
                *slot = color.rgb;
            }
        }
        let Some(max_seq) = value.custom.iter().map(|color| color.seq).max() else {
            return Self {
                ansi,
                custom: CustomColours::default(),
            };
        };
        let mut pairs = vec![None; max_seq.get()];
        for color in value.custom {
            pairs[color.seq.get() - 1] = Some(ColorPair {
                text: color.text,
                back: color.back,
            });
        }
        Self {
            ansi,
            custom: CustomColours { pairs },
        }
    }
}

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "colours", default)]
struct ColoursXml {
    #[serde(rename = "@muclient_version")]
    pub muclient_version: Option<String>,
    #[serde(rename = "@world_file_version")]
    pub world_file_version: Option<u32>,
    #[serde(rename = "@date_saved")]
    pub date_saved: Option<String>,
    pub ansi: Ansi,
    pub custom: XmlList<CustomColour>,
}

#[derive(Debug, Default, Deserialize)]
#[serde(rename = "ansi", default)]
struct Ansi {
    normal: XmlList<Colour>,
    bold: XmlList<Colour>,
}

#[derive(Debug, Deserialize)]
#[serde(rename = "colour")]
struct Colour {
    #[serde(rename = "@seq")]
    pub seq: NonZero<usize>,
    #[serde(rename = "@rgb")]
    pub rgb: RgbColor,
}

#[derive(Debug, Deserialize)]
#[serde(rename = "colour")]
struct CustomColour {
    #[serde(rename = "@seq")]
    pub seq: NonZero<usize>,
    #[allow(unused)]
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@text")]
    pub text: RgbColor,
    #[serde(rename = "@back")]
    pub back: RgbColor,
}

#[derive(Debug, Deserialize)]
#[serde(default)]
struct XmlList<T> {
    #[serde(rename = "$value")]
    pub elements: Vec<T>,
}

impl<T> Default for XmlList<T> {
    fn default() -> Self {
        Self {
            elements: Vec::new(),
        }
    }
}

impl<T> XmlList<T> {
    pub fn iter(&self) -> slice::Iter<'_, T> {
        self.elements.iter()
    }
}

impl<T> IntoIterator for XmlList<T> {
    type Item = T;
    type IntoIter = vec::IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        self.elements.into_iter()
    }
}
