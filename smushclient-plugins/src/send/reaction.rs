use fancy_regex::{Captures, Expander};
use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;
use super::sender::Sender;
use crate::regex::{Regex, RegexError};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
pub struct Reaction {
    // Note: this is at the top for Ord-deriving purposes.
    pub sequence: i16,
    pub pattern: String,
    pub send: Sender,

    pub ignore_case: bool,
    pub keep_evaluating: bool,
    pub is_regex: bool,
    pub expand_variables: bool,
    pub repeats: bool,

    pub regex: Regex,
}

impl_deref!(Reaction, Sender, send);
impl_asref!(Reaction, Sender);

impl Default for Reaction {
    fn default() -> Self {
        Self {
            sequence: Self::DEFAULT_SEQUENCE,
            pattern: String::new(),
            send: Sender::default(),
            ignore_case: false,
            keep_evaluating: false,
            is_regex: false,
            expand_variables: false,
            repeats: false,
            regex: Regex::default(),
        }
    }
}

impl Reaction {
    pub const DEFAULT_SEQUENCE: i16 = crate::constants::DEFAULT_SEQUENCE;

    pub fn expand_text<'a>(&self, buf: &'a mut Vec<u8>, captures: &Captures) -> &'a str {
        Expander {
            sub_char: '%',
            ..Default::default()
        }
        .write_expansion_vec(buf, &self.text, captures)
        .expect("expansion succeeded");
        std::str::from_utf8(buf).expect("expansion is UTF-8")
    }

    pub fn has_send(&self) -> bool {
        !self.text.is_empty() || self.send_to == SendTarget::Variable
    }

    pub fn make_regex(pattern: &str, is_regex: bool) -> Result<Regex, RegexError> {
        #[rustfmt::skip]
        fn is_special(c: char) -> bool {
            matches!(c, '\\'|'.'|'+'|'*'|'?'|'('|')'|'|'|'['|']'|'{'|'}'|'^'|'$'|'#')
        }
        if is_regex {
            Regex::new(pattern)
        } else {
            let mut buf = String::with_capacity(pattern.len() * 4);
            buf.push('^');
            for c in pattern.chars() {
                if c == '*' {
                    buf.push_str("(.*)");
                    continue;
                }
                if is_special(c) {
                    buf.push('\\');
                }
                buf.push(c);
            }
            buf.push('$');
            Regex::new(&buf)
        }
    }
}
