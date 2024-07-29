use core::str;
use smushclient_plugins::{Pad, PluginIndex, Sender};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct SendRequest<'a> {
    pub plugin: PluginIndex,
    pub line: &'a str,
    pub wildcards: Vec<&'a str>,
    pub pad: Option<Pad<'a>>,
    pub sender: &'a Sender,
    pub text: &'a str,
}
