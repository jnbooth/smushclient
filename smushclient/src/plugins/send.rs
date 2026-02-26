use mud_transformer::Output;
use smushclient_plugins::{Captures, PluginIndex, Regex, SendTarget};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct SendRequest<'a> {
    pub plugin: PluginIndex,
    pub send_to: SendTarget,
    pub echo: bool,
    pub log: bool,
    pub text: &'a str,
    pub destination: &'a str,
}

pub struct SendScriptRequest<'a> {
    pub plugin: PluginIndex,
    pub script: &'a str,
    pub label: &'a str,
    pub line: &'a str,
    pub regex: &'a Regex,
    pub wildcards: Option<Captures<'a>>,
    pub output: &'a [Output],
}
