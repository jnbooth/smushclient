use std::fmt::Write;
use std::iter::FusedIterator;
use std::{slice, vec};

use super::error::{COUNTER_LIMIT, Error};
use super::step::{Direction, Interaction, Step};

pub fn evaluate<'a>(speedwalk: &'a str, filler: &'a str) -> Iter<'a> {
    Iter {
        inner: speedwalk.as_bytes().iter(),
        filler,
    }
}

pub fn evaluate_to_string(speedwalk: &str, filler: &str) -> Result<String, Error> {
    let mut result = String::new();
    for step in evaluate(speedwalk, filler) {
        let step = step?;
        write!(result, "{step}").expect("formatting error");
    }
    Ok(result)
}

pub fn encode_to_string<'a, I: IntoIterator<Item = Step<'a>>>(iter: I) -> String {
    let mut result = String::new();
    for step in iter.into_iter().map(Step::encode) {
        write!(result, "{step}").expect("formatting error");
    }
    result
}

pub fn reverse<'a>(speedwalk: &'a str, filler: &'a str) -> Result<vec::IntoIter<Step<'a>>, Error> {
    let steps: Result<Vec<_>, _> = evaluate(speedwalk, filler)
        .map(|step| step.map(Step::reverse))
        .collect();
    let mut steps = steps?;
    steps.reverse();
    Ok(steps.into_iter())
}

pub fn reverse_to_string(speedwalk: &str) -> Result<String, Error> {
    Ok(encode_to_string(reverse(speedwalk, "")?))
}

pub struct Iter<'a> {
    inner: slice::Iter<'a, u8>,
    filler: &'a str,
}

impl<'a> Iterator for Iter<'a> {
    type Item = Result<Step<'a>, Error>;

    fn next(&mut self) -> Option<Self::Item> {
        match self.get_next() {
            Ok(Some(next)) => Some(Ok(next)),
            Ok(None) => None,
            Err(err) => {
                self.inner.nth(usize::MAX); // skip to end
                Some(Err(err))
            }
        }
    }
}

impl<'a> Iter<'a> {
    fn get_next(&mut self) -> Result<Option<Step<'a>>, Error> {
        let Some(mut c) = self.first_after_whitespace_and_comments()? else {
            return Ok(None);
        };
        let count = if c.is_ascii_digit() {
            let count = self.take_count(c)?;
            c = self
                .first_after_whitespace_and_comments()?
                .ok_or(Error::CounterWithoutAction)?;
            Some(count)
        } else {
            None
        };
        let act = match c {
            b'C' | b'c' => Some(Interaction::Close),
            b'O' | b'o' => Some(Interaction::Open),
            b'L' | b'l' => Some(Interaction::Lock),
            b'K' | b'k' => Some(Interaction::Unlock),
            _ => None,
        };
        if act.is_some() {
            if count.is_some() {
                return Err(Error::CounterWithColk);
            }
            c = match self.inner.next() {
                None | Some(b'F' | b'f') => return Err(Error::ColkWithoutDirection),
                Some(c) => *c,
            };
        }
        let dir = match c {
            b'N' | b'n' => Direction::North,
            b'S' | b's' => Direction::South,
            b'E' | b'e' => Direction::East,
            b'W' | b'w' => Direction::West,
            b'U' | b'u' => Direction::Up,
            b'D' | b'd' => Direction::Down,
            b'(' => self.take_custom()?,
            b'F' | b'f' => return Ok(Some(Step::Filler(self.filler, count))),
            _ => {
                return Err(Error::InvalidDirection(c.into()));
            }
        };
        Ok(Some(match act {
            Some(act) => Step::Interact(act, dir),
            None => Step::Move(dir, count),
        }))
    }

    fn first_after_whitespace_and_comments(&mut self) -> Result<Option<u8>, Error> {
        let mut in_comment = false;
        for &c in &mut self.inner {
            if in_comment {
                if c == b'}' {
                    in_comment = false;
                }
            } else if c == b'{' {
                in_comment = true;
            } else if !c.is_ascii_whitespace() {
                return Ok(Some(c));
            }
        }
        if in_comment {
            Err(Error::UnterminatedComment)
        } else {
            Ok(None)
        }
    }

    fn take_count(&mut self, first_digit: u8) -> Result<u8, Error> {
        let mut count = first_digit - b'0';
        while let Some(&c @ b'0'..=b'9') = self.inner.as_slice().first() {
            count = count * 10 + c - b'0';
            if count > COUNTER_LIMIT {
                return Err(Error::CounterExceedsLimit);
            }
            self.inner.next();
        }
        Ok(count)
    }

    fn take_custom(&mut self) -> Result<Direction<'a>, Error> {
        let slice = self.inner.as_slice();
        let forward_end = self
            .inner
            .position(|&c| c == b')' || c == b'/')
            .ok_or(Error::UnterminatedParenthesis)?;
        let (forward, rest) = slice.split_at(forward_end);
        if rest[0] == b')' {
            return Ok(Direction::Custom {
                forward: str::from_utf8(forward)?,
                backward: "",
            });
        }
        let backward_end = self
            .inner
            .position(|&c| c == b')')
            .ok_or(Error::UnterminatedParenthesis)?;
        Ok(Direction::Custom {
            forward: str::from_utf8(forward)?,
            backward: str::from_utf8(&rest[1..=backward_end])?,
        })
    }
}

impl FusedIterator for Iter<'_> {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn evaluate_speedwalk() {
        let speedwalk = evaluate_to_string("4n3es(nw/se)uw(say open sesame)dFLW", "filler")
            .expect("evaluation error");
        assert_eq!(
            speedwalk,
            "north\r\nnorth\r\nnorth\r\nnorth\r\neast\r\neast\r\neast\r\nsouth\r\nnw\r\nup\r\nwest\r\nsay open sesame\r\ndown\r\nfiller\r\nlock west\r\n"
        );
    }

    #[test]
    fn reverse_speedwalk() {
        let speedwalk =
            reverse_to_string("4n3es(nw/se)uw(say open sesame)dFLW").expect("evaluation error");
        assert_eq!(speedwalk, "LEFU(say open sesame)ED(se/nw)N3W4S");
    }
}
