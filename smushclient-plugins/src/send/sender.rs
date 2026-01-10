use std::sync::atomic::{AtomicU16, Ordering};

use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
#[serde(default)]
pub struct Sender {
    // Note: these two fields are at the top for Ord-deriving purposes.
    pub group: String,
    pub label: String,
    pub send_to: SendTarget,
    pub script: String,
    pub variable: String,
    pub text: String,

    pub enabled: bool,
    pub one_shot: bool,
    pub temporary: bool,
    pub omit_from_output: bool,
    pub omit_from_log: bool,

    #[serde(skip, default = "Sender::get_id")]
    pub id: u16,
    #[serde(skip)]
    pub userdata: i32,
}

impl Clone for Sender {
    fn clone(&self) -> Self {
        Self {
            group: self.group.clone(),
            label: self.label.clone(),
            send_to: self.send_to,
            script: self.script.clone(),
            variable: self.variable.clone(),
            text: self.text.clone(),
            enabled: self.enabled,
            one_shot: self.one_shot,
            temporary: self.temporary,
            omit_from_output: self.omit_from_output,
            omit_from_log: self.omit_from_log,
            id: self.id,
            userdata: self.userdata,
        }
    }

    fn clone_from(&mut self, source: &Self) {
        self.group.clone_from(&source.group);
        self.label.clone_from(&source.label);
        self.send_to = source.send_to;
        self.script.clone_from(&source.script);
        self.variable.clone_from(&source.variable);
        self.text.clone_from(&source.text);
        self.enabled = source.enabled;
        self.one_shot = source.one_shot;
        self.temporary = source.temporary;
        self.omit_from_output = source.omit_from_output;
        self.omit_from_log = source.omit_from_log;
        self.id = source.id;
        self.userdata = source.userdata;
    }
}

impl Default for Sender {
    fn default() -> Self {
        Self::new()
    }
}

impl Sender {
    pub(super) fn get_id() -> u16 {
        static ID_COUNTER: AtomicU16 = AtomicU16::new(0);
        ID_COUNTER.fetch_add(1, Ordering::Relaxed)
    }

    pub fn new() -> Self {
        Self {
            text: String::new(),
            send_to: SendTarget::World,
            label: String::new(),
            script: String::new(),
            group: String::new(),
            variable: String::new(),
            enabled: true,
            one_shot: false,
            temporary: false,
            omit_from_output: false,
            omit_from_log: false,
            id: Self::get_id(),
            userdata: 0,
        }
    }

    pub fn destination(&self) -> &str {
        if self.variable.is_empty() {
            self.label.as_str()
        } else {
            self.variable.as_str()
        }
    }

    pub fn should_echo(&self) -> bool {
        !self.omit_from_output
            && matches!(
                self.send_to,
                SendTarget::World | SendTarget::WorldImmediate | SendTarget::Speedwalk
            )
    }
}
