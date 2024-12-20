use pcre2::bytes::Captures;
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
        buf.clear();
        let mut chars = self.text.as_bytes().iter().copied();
        while let Some(c) = chars.next() {
            if c != b'%' {
                buf.push(c);
                continue;
            }
            let mut replace = 0usize;
            let mut leftover = b'0';
            for c in chars.by_ref() {
                if !c.is_ascii_digit() {
                    leftover = c;
                    break;
                }
                replace = replace * 10 + usize::from(c - b'0');
            }
            if let Some(capture) = captures.get(replace) {
                buf.extend_from_slice(capture.as_bytes());
            }
            if leftover != b'0' {
                buf.push(leftover);
            }
        }
        Regex::expect(buf)
    }

    pub fn has_send(&self) -> bool {
        !self.text.is_empty() || self.send_to == SendTarget::Variable
    }

    pub fn make_regex_pattern(pattern: &str, buf: &mut String) {
        #[rustfmt::skip]
        fn is_special(c: char) -> bool {
            matches!(c, '\\'|'.'|'+'|'*'|'?'|'('|')'|'|'|'['|']'|'{'|'}'|'^'|'$'|'#')
        }
        buf.reserve(pattern.len() * 4);
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
    }

    pub fn make_regex(pattern: &str, is_regex: bool) -> Result<Regex, RegexError> {
        if is_regex {
            Regex::new(pattern)
        } else {
            let mut buf = String::new();
            Self::make_regex_pattern(pattern, &mut buf);
            Regex::new(&buf)
        }
    }
}
