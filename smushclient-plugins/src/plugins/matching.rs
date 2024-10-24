use fancy_regex::{CaptureMatches, Captures};

use crate::send::{Reaction, SendTarget, Sender};
use crate::PluginIndex;

enum RepeatingMatch<'a, 'b, T> {
    Empty,
    Repeat(usize, SendMatch<'a, 'b, T>),
    Captures {
        plugin: PluginIndex,
        index: usize,
        iter: CaptureMatches<'a, 'b>,
        sender: &'a T,
    },
}

impl<'a, 'b, T: AsRef<Reaction>> Iterator for RepeatingMatch<'a, 'b, T> {
    type Item = SendMatch<'a, 'b, T>;

    fn next(&mut self) -> Option<Self::Item> {
        let next = match self {
            RepeatingMatch::Empty => return None,
            RepeatingMatch::Repeat(0, matched) => Some(matched.clone_uncaptured()),
            RepeatingMatch::Repeat(times, matched) => {
                *times -= 1;
                return Some(matched.clone_uncaptured());
            }
            RepeatingMatch::Captures {
                plugin,
                index,
                iter,
                sender,
            } => match iter.next() {
                Some(Ok(captures)) => {
                    let sender: &T = sender;
                    let reaction = sender.as_ref();
                    return Some(SendMatch {
                        plugin: *plugin,
                        index: *index,
                        sender,
                        text: &reaction.text,
                        captures: Some(captures),
                    });
                }
                _ => None,
            },
        };
        *self = RepeatingMatch::Empty;
        next
    }
}

pub struct SendMatches<'a, 'b, I, T> {
    done: bool,
    inner: I,
    line: &'b str,
    repeating: RepeatingMatch<'a, 'b, T>,
}

impl<'a, 'b, I, T> SendMatches<'a, 'b, I, T> {
    pub fn find(iter: I, line: &'b str) -> Self {
        Self {
            done: false,
            inner: iter,
            line,
            repeating: RepeatingMatch::Empty,
        }
    }
}

impl<'a, 'b, I, T> Iterator for SendMatches<'a, 'b, I, T>
where
    I: Iterator<Item = (PluginIndex, usize, &'a T)>,
    T: AsRef<Reaction>,
{
    type Item = SendMatch<'a, 'b, T>;

    fn next(&mut self) -> Option<Self::Item> {
        if let Some(repeated) = self.repeating.next() {
            return Some(repeated);
        }
        if self.done {
            return None;
        }
        for (plugin, index, sender) in &mut self.inner {
            let reaction = sender.as_ref();
            let count = if reaction.repeats {
                reaction.regex.find_iter(self.line).count()
            } else {
                1
            };
            if reaction.script.is_empty() && !reaction.text.contains('$') {
                let val = SendMatch {
                    plugin,
                    index,
                    sender,
                    text: &reaction.text,
                    captures: None,
                };
                if count > 1 {
                    self.repeating = RepeatingMatch::Repeat(count - 2, val.clone_uncaptured());
                }
                return Some(val);
            }
            let iter = reaction.regex.captures_iter(self.line);
            self.repeating = RepeatingMatch::Captures {
                plugin,
                index,
                iter,
                sender,
            };
            if let Some(repeated) = self.repeating.next() {
                return Some(repeated);
            }
        }
        None
    }
}

#[derive(Debug)]
pub struct SendMatch<'a, 'b, T> {
    pub plugin: PluginIndex,
    pub index: usize,
    pub sender: &'a T,
    text: &'a str,
    captures: Option<Captures<'b>>,
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
                captures.expand(self.text, buf);
                buf.as_str()
            }
            None => self.text,
        }
    }

    pub fn wildcards(&self) -> Vec<&'b str> {
        let Some(ref captures) = &self.captures else {
            return Vec::new();
        };
        let mut wildcards = Vec::with_capacity(captures.len() - 1);
        let mut iter = captures.iter();
        iter.next(); // skip the all-encompassing capture
        for capture in iter.flatten() {
            wildcards.push(capture.as_str());
        }
        wildcards
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
            || !sender.script.is_empty()
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
        let aliases = &[(1, 0, &alias)];
        let alias_iter = aliases.iter().copied().filter(|(_, _, alias)| {
            alias.enabled && matches!(alias.regex.is_match(opts.line), Ok(true))
        });
        let outputs: Vec<_> = SendMatches::find(alias_iter, opts.line)
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
