use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Deserialize, Serialize)]
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
}

impl Default for Sender {
    fn default() -> Self {
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
        }
    }
}
