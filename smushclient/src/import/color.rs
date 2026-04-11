use std::num::NonZero;
use std::{iter, slice};

use mud_transformer::opt::mxp::RgbColor;
use serde::Deserialize;
use smushclient_plugins::xml::XmlTrigger;

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

    pub fn apply_to_trigger(&self, trigger: &mut XmlTrigger) {
        let Some(pair) = self.get(trigger.custom_colour) else {
            return;
        };
        trigger.other_back_colour.get_or_insert(pair.text);
        trigger.other_text_colour.get_or_insert(pair.back);
    }
}

impl From<&ColoursXml> for CustomColours {
    fn from(value: &ColoursXml) -> Self {
        let Some(max_seq) = value.custom.elements.iter().map(|color| color.seq).max() else {
            return Self::default();
        };
        let max_seq = max_seq.get();
        let mut pairs = vec![None; max_seq];
        for color in &value.custom.elements {
            pairs[color.seq.get() - 1] = Some(color.into());
        }
        Self { pairs }
    }
}

#[derive(Debug, Deserialize)]
#[serde(from = "ColoursXml")]
pub(super) struct Colours {
    pub ansi: [RgbColor; 16],
    pub custom: CustomColours,
}

impl From<[RgbColor; 16]> for Colours {
    fn from(ansi: [RgbColor; 16]) -> Self {
        Self {
            ansi,
            custom: CustomColours::default(),
        }
    }
}

impl From<ColoursXml> for Colours {
    fn from(value: ColoursXml) -> Self {
        Self::from(&value)
    }
}

impl From<&ColoursXml> for Colours {
    fn from(value: &ColoursXml) -> Self {
        let mut ansi = RgbColor::XTERM_16;
        ansi.extend(&value.ansi);
        Self {
            ansi,
            custom: value.into(),
        }
    }
}

#[derive(Clone, Debug, Default, Deserialize)]
#[serde(rename = "colours", default)]
pub(crate) struct ColoursXml {
    #[serde(rename = "@muclient_version")]
    pub muclient_version: Option<String>,
    #[serde(rename = "@world_file_version")]
    pub world_file_version: Option<u32>,
    #[serde(rename = "@date_saved")]
    pub date_saved: Option<String>,
    pub ansi: Ansi,
    pub custom: XmlList<CustomColour>,
}

impl ColoursXml {
    pub fn len(&self) -> usize {
        self.ansi.len() + self.custom.elements.len()
    }

    pub fn append(&mut self, other: &mut Self) {
        self.ansi.append(&mut other.ansi);
        self.custom.append(&mut other.custom);
    }
}

#[derive(Clone, Debug, Default, Deserialize)]
#[serde(rename = "ansi", default)]
pub(crate) struct Ansi {
    normal: XmlList<Colour>,
    bold: XmlList<Colour>,
}

impl Ansi {
    fn len(&self) -> usize {
        self.normal.elements.len() + self.bold.elements.len()
    }

    fn append(&mut self, other: &mut Self) {
        self.normal.append(&mut other.normal);
        self.bold.append(&mut other.bold);
    }
}

type CopiedColours<'a> = iter::Copied<slice::Iter<'a, Colour>>;

impl<'a> IntoIterator for &'a Ansi {
    type Item = Colour;

    type IntoIter =
        iter::Chain<CopiedColours<'a>, iter::Map<CopiedColours<'a>, fn(Colour) -> Colour>>;

    fn into_iter(self) -> Self::IntoIter {
        const fn normalize_bold(Colour { seq, rgb }: Colour) -> Colour {
            Colour {
                seq: seq.saturating_add(8),
                rgb,
            }
        }
        self.normal.elements.iter().copied().chain(
            self.bold
                .elements
                .iter()
                .copied()
                .map(normalize_bold as fn(Colour) -> Colour),
        )
    }
}

#[derive(Copy, Clone, Debug, Deserialize)]
#[serde(rename = "colour")]
pub(crate) struct Colour {
    #[serde(rename = "@seq")]
    pub seq: NonZero<usize>,
    #[serde(rename = "@rgb")]
    pub rgb: RgbColor,
}

impl Extend<Colour> for [RgbColor] {
    fn extend<T: IntoIterator<Item = Colour>>(&mut self, iter: T) {
        for color in iter {
            if let Some(slot) = self.get_mut(color.seq.get() - 1) {
                *slot = color.rgb;
            }
        }
    }
}

#[derive(Clone, Debug, Deserialize)]
#[serde(rename = "colour")]
pub(crate) struct CustomColour {
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

impl From<&CustomColour> for ColorPair {
    fn from(value: &CustomColour) -> Self {
        Self {
            text: value.text,
            back: value.back,
        }
    }
}

#[derive(Clone, Debug, Deserialize)]
#[serde(default)]
pub(crate) struct XmlList<T> {
    #[serde(rename = "$value")]
    pub elements: Vec<T>,
}

impl<T> XmlList<T> {
    pub fn append(&mut self, other: &mut Self) {
        self.elements.append(&mut other.elements);
    }
}

impl<T> Default for XmlList<T> {
    fn default() -> Self {
        Self {
            elements: Vec::new(),
        }
    }
}
