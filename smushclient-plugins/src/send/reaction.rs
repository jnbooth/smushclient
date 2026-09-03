use std::borrow::Cow;
use std::fmt::Write as _;
use std::iter;
#[cfg(not(feature = "send"))]
use std::rc::Rc;
use std::str::CharIndices;
#[cfg(feature = "send")]
use std::sync::{Arc as Rc, LazyLock};

use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;
use super::sender::Sender;
use crate::regex::{Captures, Regex, RegexBuilder, RegexError};

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize)]
#[serde(try_from = "ReactionSerde")]
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

    pub regex: Rc<Regex>,
}

impl_deref!(Reaction, Sender, send);
impl_asref!(Reaction, Sender);

impl Default for Reaction {
    fn default() -> Self {
        fn make_default_regex_rc() -> Rc<Regex> {
            Rc::new(Regex::new("^$").unwrap())
        }

        #[cfg(feature = "send")]
        static DEFAULT_REGEX: LazyLock<Rc<Regex>> = LazyLock::new(make_default_regex_rc);
        #[cfg(not(feature = "send"))]
        thread_local! {
            static DEFAULT_REGEX: Rc<Regex> = make_default_regex_rc();
        }

        Self {
            sequence: Self::DEFAULT_SEQUENCE,
            pattern: String::new(),
            send: Sender::default(),

            ignore_case: false,
            keep_evaluating: false,
            is_regex: false,
            expand_variables: false,
            repeats: false,

            #[cfg(feature = "send")]
            regex: DEFAULT_REGEX.clone(),
            #[cfg(not(feature = "send"))]
            regex: DEFAULT_REGEX.with(Rc::clone),
        }
    }
}

impl Reaction {
    pub const DEFAULT_SEQUENCE: i16 = 100;

    fn set_regex(&mut self, regex: Regex) {
        if let Some(rc) = Rc::get_mut(&mut self.regex) {
            *rc = regex;
        } else {
            self.regex = Rc::new(regex);
        }
    }

    pub fn set_ignore_case(&mut self, ignore_case: bool) -> Result<(), RegexError> {
        self.set_regex(Self::make_regex(&self.pattern, self.is_regex, ignore_case)?);
        self.ignore_case = ignore_case;
        Ok(())
    }

    pub fn set_is_regex(&mut self, is_regex: bool) -> Result<(), RegexError> {
        self.set_regex(Self::make_regex(&self.pattern, is_regex, self.ignore_case)?);
        self.is_regex = is_regex;
        Ok(())
    }

    pub fn set_pattern(&mut self, pattern: String) -> Result<(), RegexError> {
        self.set_regex(Self::make_regex(&pattern, self.is_regex, self.ignore_case)?);
        self.pattern = pattern;
        Ok(())
    }

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

        let len = buf.len();
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
                None => return &buf[len..],
            };
        }
        buf.push_str(&self.text[start..]);
        &buf[len..]
    }

    pub fn expand_text_captureless<'a>(&self, buf: &'a mut String) -> &'a str {
        let len = buf.len();
        let mut iter = self.text.char_indices();
        let Some((i, _)) = iter.find(|(_, c)| *c == '%') else {
            buf.push_str(&self.text);
            return &buf[len..];
        };
        buf.push_str(&self.text[..i]);
        while let Some((start, _)) = iter.find(|(_, c)| !c.is_numeric()) {
            if !iter.any(|(_, c)| c == '%') {
                buf.push_str(&self.text[start..]);
                break;
            }
        }
        &buf[len..]
    }

    pub fn has_send(&self) -> bool {
        !self.text.is_empty() || self.send_to == SendTarget::Variable
    }

    pub fn make_regex_pattern(pattern: &str) -> String {
        const SPECIAL_CHARS: &str = "\\.+?()|[]{}^$#";
        let mut buf = String::with_capacity(pattern.len() * 4);
        buf.push('^');
        for c in pattern.chars() {
            if c == '*' {
                buf.push_str("(.*)");
                continue;
            }
            if let Ok(ch) = u8::try_from(c)
                && ch.is_ascii_control()
            {
                write!(buf, "\\x{ch:02X}").unwrap();
                continue;
            }
            if SPECIAL_CHARS.contains(c) {
                buf.push('\\');
            }
            buf.push(c);
        }
        buf.push('$');
        buf
    }

    pub fn make_regex(
        pattern: &str,
        is_regex: bool,
        ignore_case: bool,
    ) -> Result<Regex, RegexError> {
        let mut builder = RegexBuilder::new();
        builder.caseless(ignore_case);
        if is_regex {
            builder.build(pattern)
        } else {
            builder.build(&Self::make_regex_pattern(pattern))
        }
    }
}

#[derive(Deserialize, Serialize)]
struct ReactionSerde<'a> {
    sequence: i16,
    pattern: Cow<'a, str>,
    send: Cow<'a, Sender>,

    ignore_case: bool,
    keep_evaluating: bool,
    is_regex: bool,
    expand_variables: bool,
    repeats: bool,

    #[serde(borrow)]
    regex: Cow<'a, str>,
}

impl<'a> From<&'a Reaction> for ReactionSerde<'a> {
    fn from(value: &'a Reaction) -> Self {
        let &Reaction {
            sequence,
            ref pattern,
            ref send,
            ignore_case,
            keep_evaluating,
            is_regex,
            expand_variables,
            repeats,
            ref regex,
        } = value;
        Self {
            sequence,
            pattern: Cow::Borrowed(pattern),
            send: Cow::Borrowed(send),
            ignore_case,
            keep_evaluating,
            is_regex,
            expand_variables,
            repeats,
            regex: Cow::Borrowed(regex.as_str()),
        }
    }
}

impl<'a> TryFrom<ReactionSerde<'a>> for Reaction {
    type Error = RegexError;

    fn try_from(value: ReactionSerde<'a>) -> Result<Self, Self::Error> {
        let ReactionSerde {
            sequence,
            pattern,
            send,
            ignore_case,
            keep_evaluating,
            is_regex,
            expand_variables,
            repeats,
            regex,
        } = value;
        Ok(Self {
            sequence,
            pattern: pattern.into_owned(),
            send: send.into_owned(),
            ignore_case,
            keep_evaluating,
            is_regex,
            expand_variables,
            repeats,
            regex: Rc::new(RegexBuilder::new().caseless(ignore_case).build(&regex)?),
        })
    }
}

impl Serialize for Reaction {
    fn serialize<S: serde::Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        ReactionSerde::from(self).serialize(serializer)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn expand() {
        let pattern = "(.e).(.)(.*)q(\\d+)".to_owned();
        let mut reaction = Reaction {
            regex: Rc::new(Regex::new(&pattern).unwrap()),
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

    #[test]
    fn caseless() {
        let mut reaction = Reaction::default();
        reaction
            .set_pattern("^flyto (Ankh.Morpork|AM)$".to_owned())
            .unwrap();
        reaction.set_is_regex(true).unwrap();
        reaction.set_ignore_case(true).unwrap();
        assert!(reaction.regex.captures_iter("flyto am").next().is_some());
    }
}
