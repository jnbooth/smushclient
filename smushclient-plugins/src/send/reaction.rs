use std::iter;
use std::str::CharIndices;

use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;
use super::sender::Sender;
use crate::regex::{Captures, Regex, RegexError};

#[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
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

impl Clone for Reaction {
    fn clone(&self) -> Self {
        Self {
            sequence: self.sequence,
            pattern: self.pattern.clone(),
            send: self.send.clone(),
            ignore_case: self.ignore_case,
            keep_evaluating: self.keep_evaluating,
            is_regex: self.is_regex,
            expand_variables: self.expand_variables,
            repeats: self.repeats,
            regex: self.regex.clone(),
        }
    }

    fn clone_from(&mut self, source: &Self) {
        self.sequence = source.sequence;
        self.pattern.clone_from(&source.pattern);
        self.send.clone_from(&source.send);
        self.ignore_case = source.ignore_case;
        self.keep_evaluating = source.keep_evaluating;
        self.is_regex = source.is_regex;
        self.expand_variables = source.expand_variables;
        self.repeats = source.repeats;
        self.regex.clone_from(&source.regex);
    }
}

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
    pub const DEFAULT_SEQUENCE: i16 = 100;

    pub fn expand_text<'a>(&self, buf: &'a mut String, captures: &Captures) -> &'a str {
        #[inline]
        fn collect_int(iter: &mut iter::Peekable<CharIndices>, max: usize) -> Option<usize> {
            let mut value = 0;
            let mut valid = false;
            while let Some((_, c)) = iter.next_if(|(_, c)| c.is_ascii_digit()) {
                valid = true;
                if value < max {
                    value = value * 10 + (c as usize - '0' as usize);
                }
            }
            valid.then_some(value)
        }

        #[inline]
        fn collect_group<'a>(
            iter: &mut iter::Peekable<CharIndices>,
            captures: &'a Captures,
            max: usize,
        ) -> Option<&'a str> {
            let Some(group) = collect_int(iter, max) else {
                iter.next_if(|(_, c)| *c == '%')?;
                return Some("%");
            };
            Some(match captures.get(group) {
                Some(capture) => capture.as_str(),
                None => "",
            })
        }

        buf.clear();
        buf.reserve(self.text.len());
        let captures_len = captures.len();
        let mut start = 0;

        let mut iter = self.text.char_indices().peekable();

        while let Some((i, c)) = iter.next() {
            if c != '%' {
                continue;
            }
            let Some(group) = collect_group(&mut iter, captures, captures_len) else {
                continue;
            };
            buf.push_str(&self.text[start..i]);
            buf.push_str(group);
            start = match iter.peek() {
                Some((start, _)) => *start,
                None => return buf.as_str(),
            };
        }
        buf.push_str(&self.text[start..]);
        buf.as_str()
    }

    pub fn has_send(&self) -> bool {
        !self.text.is_empty() || self.send_to == SendTarget::Variable
    }

    pub fn make_regex_pattern(pattern: &str, buf: &mut String) {
        const SPECIAL_CHARS: &str = "\\.+?()|[]{}^$#";

        buf.reserve(pattern.len() * 4);
        buf.push('^');
        for c in pattern.chars() {
            if c == '*' {
                buf.push_str("(.*)");
                continue;
            }
            if SPECIAL_CHARS.contains(c) {
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_expand() {
        let pattern = "(.e).(.)(.*)q(\\d+)".to_owned();
        let mut reaction = Reaction {
            regex: Regex::new(&pattern).unwrap(),
            pattern,
            ..Default::default()
        };
        reaction.text = "z%c%%%1 %3%2%20%4h".to_owned();
        let captures = reaction
            .regex
            .captures_iter("abedegeq55!")
            .next()
            .unwrap()
            .unwrap();
        let mut buf = String::new();
        let expanded = reaction.expand_text(&mut buf, &captures);
        assert_eq!(expanded, "z%c%be gee55h");
    }
}
