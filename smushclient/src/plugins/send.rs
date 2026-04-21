#[cfg(not(feature = "async"))]
use std::rc::Rc;
#[cfg(feature = "async")]
use std::sync::Arc as Rc;

use mud_transformer::output::Output;
use smushclient_plugins::{Captures, PluginIndex, Reaction, Regex, SendTarget};

#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
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

#[derive(Debug)]
pub struct SendRequestBuffer<'a> {
    line: &'a str,
    output: &'a [Output],
    text: String,
    destination: String,
    regex: Option<Rc<Regex>>,
}

impl<'a> SendRequestBuffer<'a> {
    pub fn new(line: &'a str, output: &'a [Output]) -> Self {
        Self {
            line,
            output,
            text: String::new(),
            destination: String::new(),
            regex: None,
        }
    }
}

impl SendRequestBuffer<'_> {
    pub fn send_request<'a>(
        &'a mut self,
        plugin: PluginIndex,
        reaction: &Reaction,
        enable_scripts: bool,
        captures: Option<&Captures<'_>>,
    ) -> Option<SendRequest<'a>> {
        if !enable_scripts && reaction.send_to.is_script() {
            return None;
        }
        self.text.clear();
        match captures {
            Some(captures) => reaction.expand_text(&mut self.text, captures),
            None => reaction.expand_text_captureless(&mut self.text),
        };
        reaction.destination().clone_into(&mut self.destination);
        Some(SendRequest {
            plugin,
            send_to: reaction.send_to,
            echo: !reaction.omit_from_output,
            log: !reaction.omit_from_log,
            text: &self.text,
            destination: &self.destination,
        })
    }

    pub fn send_script_request<'a>(
        &'a mut self,
        plugin: PluginIndex,
        reaction: &Reaction,
    ) -> Option<SendScriptRequest<'a>> {
        if reaction.script.is_empty() {
            return None;
        }
        reaction.script.clone_into(&mut self.text);
        reaction.label.clone_into(&mut self.destination);
        self.regex = Some(reaction.regex.clone());
        Some(SendScriptRequest {
            plugin,
            script: &self.text,
            label: &self.destination,
            line: self.line,
            regex: self.regex.as_deref().unwrap(),
            wildcards: None,
            output: self.output,
        })
    }
}
