use std::borrow::Cow;
use std::fmt;
use std::marker::PhantomData;

use serde::de::{Error as _, SeqAccess, Visitor};
use serde::{Deserialize, Deserializer};
use smushclient_plugins::xml::{XmlAlias, XmlTimer, XmlTrigger, XmlVec};
use smushclient_plugins::{Alias, ImportError, RegexError, Timer, Trigger};

use super::color::{ColoursXml, CustomColours};
use super::world::MuClient;
use crate::client::XmlVariable;
use crate::world::XmlKey;

#[derive(Debug, Deserialize)]
#[allow(clippy::large_enum_variant)]
enum XmlImport<'a> {
    #[serde(borrow)]
    MuClient(MuClient<'a>),
    // World(XmlWorld),
    #[serde(borrow)]
    Triggers(XmlVec<XmlTrigger<'a>>),
    #[serde(borrow)]
    Aliases(XmlVec<XmlAlias<'a>>),
    #[serde(borrow)]
    Timers(XmlVec<XmlTimer<'a>>),
    Colours(ColoursXml),
    #[serde(borrow)]
    Keypad(XmlVec<XmlKey<'a>>),
    #[serde(borrow)]
    Variables(XmlVec<XmlVariable<'a>>),
    #[serde(borrow)]
    Script(Cow<'a, str>),
    // Macros(),
    // Printing(),
    // Comment(),
    // Include(),
    // Plugin(),
    // Script(),
    #[serde(other)]
    Unknown,
}

pub(crate) struct Imports<'a> {
    pub aliases: Vec<Alias>,
    pub colours: ColoursXml,
    pub timers: Vec<Timer>,
    pub triggers: Vec<Trigger>,
    pub variables: Vec<XmlVariable<'a>>,
    pub keys: Vec<XmlKey<'a>>,
    pub script: String,
}

impl<'a> Imports<'a> {
    pub fn from_xml(xml: &'a str) -> Result<Self, ImportError> {
        let mut deserializer = quick_xml::de::Deserializer::from_str(xml);
        let mut imports = XmlImports::deserialize(&mut deserializer)?;
        let custom_colours = CustomColours::from(&imports.colours);
        for trigger in &mut imports.triggers {
            custom_colours.apply_to_trigger(trigger);
        }
        Ok(Self::try_from(XmlImports::deserialize(&mut deserializer)?)?)
    }
}

impl<'de> Deserialize<'de> for Imports<'de> {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        Self::try_from(XmlImports::deserialize(deserializer)?).map_err(D::Error::custom)
    }
}

impl<'a> TryFrom<XmlImports<'a>> for Imports<'a> {
    type Error = RegexError;

    fn try_from(value: XmlImports<'a>) -> Result<Self, Self::Error> {
        let XmlImports {
            aliases,
            colours,
            timers,
            triggers,
            variables,
            keys,
            script,
        } = value;
        Ok(Self {
            aliases: aliases
                .into_iter()
                .map(Alias::try_from)
                .collect::<Result<_, _>>()?,
            colours,
            timers: timers.into_iter().map(Timer::from).collect(),
            triggers: triggers
                .into_iter()
                .map(Trigger::try_from)
                .collect::<Result<_, _>>()?,
            variables,
            keys,
            script,
        })
    }
}

#[derive(Debug, Default)]
struct XmlImports<'a> {
    aliases: Vec<XmlAlias<'a>>,
    colours: ColoursXml,
    timers: Vec<XmlTimer<'a>>,
    triggers: Vec<XmlTrigger<'a>>,
    variables: Vec<XmlVariable<'a>>,
    keys: Vec<XmlKey<'a>>,
    script: String,
}

impl<'a> XmlImports<'a> {
    pub fn append(&mut self, xml: &mut XmlImport<'a>) {
        match xml {
            XmlImport::Aliases(aliases) => self.aliases.append(aliases),
            XmlImport::Colours(colours) => self.colours.append(colours),
            XmlImport::Keypad(keys) => self.keys.append(keys),
            XmlImport::Script(script) => self.script.push_str(script),
            XmlImport::Timers(timers) => self.timers.append(timers),
            XmlImport::Triggers(triggers) => self.triggers.append(triggers),
            XmlImport::Variables(vars) => self.variables.append(vars),
            XmlImport::Unknown => (),
            XmlImport::MuClient(MuClient {
                aliases,
                colours,
                include: _,
                keypad,
                timers,
                triggers,
                variables,
                world: _,
            }) => {
                self.aliases.append(aliases);
                self.colours.append(colours);
                self.keys.append(keypad);
                self.timers.append(timers);
                self.triggers.append(triggers);
                self.variables.append(variables);
            }
        }
    }
}

impl<'de> Deserialize<'de> for XmlImports<'de> {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct XmlVisitor<'de> {
            marker: PhantomData<&'de ()>,
        }

        impl<'de> Visitor<'de> for XmlVisitor<'de> {
            type Value = XmlImports<'de>;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("a sequence")
            }

            fn visit_seq<A: SeqAccess<'de>>(self, mut seq: A) -> Result<Self::Value, A::Error> {
                let mut imports = Self::Value::default();
                while let Some(mut value) = seq.next_element()? {
                    imports.append(&mut value);
                }
                Ok(imports)
            }
        }

        let visitor = XmlVisitor {
            marker: PhantomData,
        };
        deserializer.deserialize_seq(visitor)
    }
}
