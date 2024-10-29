use std::mem;

use fancy_regex::{CaptureMatches, Captures, Expander};

use crate::send::{Reaction, SendTarget, Sender};
use crate::PluginIndex;

#[derive(Debug)]
pub enum SendMatchIter<'a, 'b, T> {
    Empty,
    Repeat(usize, SendMatch<'a, 'b, T>),
    Captures {
        plugin: PluginIndex,
        index: usize,
        iter: CaptureMatches<'a, 'b>,
        sender: &'a T,
    },
}

pub trait SendMatchIterable: Sized {
    fn matches<'a, 'b>(
        &'a self,
        plugin: PluginIndex,
        index: usize,
        line: &'b str,
    ) -> SendMatchIter<'a, 'b, Self>;
}

impl<T: AsRef<Reaction> + Sized> SendMatchIterable for T {
    fn matches<'a, 'b>(
        &'a self,
        plugin: PluginIndex,
        index: usize,
        line: &'b str,
    ) -> SendMatchIter<'a, 'b, Self> {
        let reaction = self.as_ref();
        if !reaction.script.is_empty() || reaction.text.contains('%') {
            return SendMatchIter::Captures {
                plugin,
                index,
                iter: reaction.regex.captures_iter(line),
                sender: self,
            };
        }
        let count = if reaction.repeats {
            reaction.regex.find_iter(line).count()
        } else {
            1
        };
        let matched = SendMatch {
            plugin,
            index,
            sender: self,
            text: &reaction.text,
            captures: None,
        };
        SendMatchIter::Repeat(count, matched)
    }
}

impl<'a, 'b, T: AsRef<Reaction>> Iterator for SendMatchIter<'a, 'b, T> {
    type Item = SendMatch<'a, 'b, T>;

    fn next(&mut self) -> Option<Self::Item> {
        let mut buf = Self::Empty;
        mem::swap(self, &mut buf);
        match buf {
            Self::Empty => None,
            Self::Repeat(1, matched) => Some(matched),
            Self::Repeat(times, matched) => {
                *self = Self::Repeat(times - 1, matched.clone_uncaptured());
                Some(matched)
            }
            Self::Captures {
                plugin,
                index,
                mut iter,
                sender,
            } => match iter.next() {
                Some(Ok(captures)) => {
                    let sender: &T = sender;
                    let reaction = sender.as_ref();
                    let matched = SendMatch {
                        plugin,
                        index,
                        sender,
                        text: &reaction.text,
                        captures: Some(captures),
                    };
                    *self = Self::Captures {
                        plugin,
                        index,
                        iter,
                        sender,
                    };
                    Some(matched)
                }
                _ => None,
            },
        }
    }
}

#[derive(Debug)]
pub struct SendMatch<'a, 'b, T> {
    pub plugin: PluginIndex,
    pub index: usize,
    pub sender: &'a T,
    text: &'a str,
    pub captures: Option<Captures<'b>>,
}

impl<'a, 'b, T> SendMatch<'a, 'b, T> {
    pub fn text<'c, 'd>(&self, buf: &'c mut String) -> &'d str
    where
        'a: 'd,
        'c: 'd,
    {
        match &self.captures {
            Some(ref captures) => {
                buf.clear();
                Expander {
                    sub_char: '%',
                    ..Default::default()
                }
                .append_expansion(buf, self.text, captures);
                buf.as_str()
            }
            None => self.text,
        }
    }

    fn clone_uncaptured(&self) -> Self {
        Self {
            plugin: self.plugin,
            index: self.index,
            sender: self.sender,
            text: self.text,
            captures: None,
        }
    }
}

impl<'a, 'b, T: AsRef<Sender>> SendMatch<'a, 'b, T> {
    pub fn has_send(&self) -> bool {
        let sender = self.sender.as_ref();
        let send_to = sender.send_to;
        !send_to.ignore_empty()
            || !self.text.is_empty()
            || (send_to == SendTarget::Variable && !sender.variable.is_empty())
    }
}
#[cfg(test)]
mod tests {
    use crate::Alias;

    use super::*;

    struct Opts<'a> {
        pattern: &'a str,
        is_regex: bool,
        repeats: bool,
        text: &'a str,
        line: &'a str,
        expect: &'a [&'a str],
    }

    #[allow(clippy::needless_pass_by_value)]
    fn test_match(opts: Opts) {
        let mut alias = Alias::default();
        alias.reaction.regex = Reaction::make_regex(opts.pattern, opts.is_regex).unwrap();
        alias.repeats = opts.repeats;
        alias.text = opts.text.to_owned();
        let mut buf = String::new();
        let outputs: Vec<_> = alias
            .matches(1, 0, opts.line)
            .map(move |send| send.text(&mut buf).to_owned())
            .collect();
        let expect: Vec<_> = opts.expect.iter().map(ToOwned::to_owned).collect();
        assert_eq!(outputs, expect, "{:?}", alias.regex);
    }

    #[test]
    fn matches_no_match() {
        test_match(Opts {
            pattern: "a*e",
            is_regex: true,
            repeats: false,
            text: "abcdeae",
            line: "b",
            expect: &[],
        });
    }

    #[test]
    fn matches_no_regex() {
        test_match(Opts {
            pattern: "a*e",
            is_regex: false,
            repeats: false,
            text: "0",
            line: "abcdeae",
            expect: &["0"],
        });
    }

    #[test]
    fn matches_regex() {
        test_match(Opts {
            pattern: "a.*e",
            is_regex: true,
            repeats: false,
            text: "0",
            line: "0abcdefgae",
            expect: &["0"],
        });
    }

    #[test]
    fn matches_captures() {
        test_match(Opts {
            pattern: "^(?>tiny |small |medium |large |huge |enormous )(.+) stencil(.+)$",
            is_regex: true,
            repeats: false,
            text: "a $2 $3 \\$1",
            line: "small green stencil!",
            expect: &["a !  \\green"],
        });
    }

    #[test]
    fn matches_repeat() {
        test_match(Opts {
            pattern: "a([^ ]*)e",
            is_regex: true,
            repeats: true,
            text: "f$2$1",
            line: "abcde ade ae",
            expect: &["fbcd", "fd", "f"],
        });
    }
}
