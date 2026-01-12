use std::sync::atomic::{AtomicU16, Ordering};

use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
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

    pub fn destination(&self) -> &String {
        if self.variable.is_empty() {
            &self.label
        } else {
            &self.variable
        }
    }
}
