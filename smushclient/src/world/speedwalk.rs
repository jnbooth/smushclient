use std::error::Error;
use std::fmt;
use std::iter::Peekable;

const COUNTER_LIMIT: usize = 99;

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum SpeedwalkError {
    UnterminatedComment,
    CounterExceedsLimit,
    CounterWithoutAction,
    CounterWithColk,
    ColkWithoutDirection,
    UnterminatedParenthesis,
    InvalidDirection(char),
}

impl fmt::Display for SpeedwalkError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Self::UnterminatedComment => f.write_str("Comment code of '{' not terminated by a '}'"),
            Self::CounterExceedsLimit => write!(f, "Speed walk counter exceeds {COUNTER_LIMIT}"),
            Self::CounterWithoutAction => {
                f.write_str("Speed walk counter not followed by an action")
            }
            Self::CounterWithColk => {
                f.write_str("Action code of C, O, L or K must not follow a speed walk count (1-99)")
            }
            Self::ColkWithoutDirection => {
                f.write_str("Action code of C, O, L or K must be followed by a direction")
            }
            Self::UnterminatedParenthesis => {
                f.write_str("Action code of '(' not terminated by a ')'")
            }
            Self::InvalidDirection(c) => write!(
                f,
                "Invalid direction '{c}' in speed walk, must be N, S, E, W, U, D, F, or (something)"
            ),
        }
    }
}

impl Error for SpeedwalkError {}

pub fn evaluate_speedwalk<I>(iter: I, filler: &str) -> Result<String, SpeedwalkError>
where
    I: IntoIterator<Item = char>,
{
    let mut result = String::new();
    let mut direction_buf = String::new();
    let mut iter = iter.into_iter().peekable();
    loop {
        take_whitespace_and_comments(&mut iter)?;
        let count = take_count(&mut iter)?;
        take_whitespace_and_comments(&mut iter)?;
        let Some(mut c) = iter.next() else {
            if count.is_some() {
                return Err(SpeedwalkError::CounterWithoutAction);
            }
            return Ok(result);
        };
        if let Some(colk) = get_colk(c) {
            if count.is_some() {
                return Err(SpeedwalkError::CounterWithColk);
            }
            result.push_str(colk);
            take_whitespace_and_comments(&mut iter)?;
            c = match iter.next() {
                None | Some('F' | 'f') => return Err(SpeedwalkError::ColkWithoutDirection),
                Some(c) => c,
            };
        }
        let dir = match c {
            'N' | 'n' => "north",
            'S' | 's' => "south",
            'E' | 'e' => "east",
            'W' | 'w' => "west",
            'U' | 'u' => "up",
            'D' | 'd' => "down",
            'F' | 'f' => filler,
            '(' => take_parenthesized(&mut iter, &mut direction_buf)?,
            _ => return Err(SpeedwalkError::InvalidDirection(c)),
        };
        let count = count.unwrap_or(1);
        result.reserve((dir.len() + 2) * count);
        for _ in 0..count {
            result.push_str(dir);
            result.push_str("\r\n");
        }
    }
}

fn take_whitespace_and_comments<I: Iterator<Item = char>>(
    iter: &mut Peekable<I>,
) -> Result<(), SpeedwalkError> {
    loop {
        while iter.next_if(char::is_ascii_whitespace).is_some() {}
        if iter.peek() != Some(&'{') {
            return Ok(());
        }
        if !iter.any(|c| c == '}') {
            return Err(SpeedwalkError::UnterminatedComment);
        }
    }
}

fn take_count<I: Iterator<Item = char>>(
    iter: &mut Peekable<I>,
) -> Result<Option<usize>, SpeedwalkError> {
    let mut matched = false;
    let mut count = 0;
    while let Some(digit) = iter.next_if(char::is_ascii_digit) {
        matched = true;
        count = count * 10 + (digit as usize - '0' as usize);
        if count > COUNTER_LIMIT {
            return Err(SpeedwalkError::CounterExceedsLimit);
        }
    }
    if matched { Ok(Some(count)) } else { Ok(None) }
}

fn get_colk(c: char) -> Option<&'static str> {
    match c {
        'C' | 'c' => Some("close "),
        'O' | 'o' => Some("open "),
        'L' | 'l' => Some("lock "),
        'K' | 'k' => Some("unlock "),
        _ => None,
    }
}

fn take_parenthesized<I>(mut iter: I, buf: &mut String) -> Result<&str, SpeedwalkError>
where
    I: Iterator<Item = char>,
{
    buf.clear();
    let mut terminated = false;
    for c in &mut iter {
        match c {
            ')' => {
                terminated = true;
                break;
            }
            '/' => {
                terminated = iter.any(|c| c == ')');
                break;
            }
            _ => buf.push(c),
        }
    }
    if !terminated {
        return Err(SpeedwalkError::UnterminatedParenthesis);
    }
    Ok(buf.as_str())
}

#[cfg(test)]
mod tests {
    #[test]
    fn evaluate_speedwalk() {
        let speedwalk =
            super::evaluate_speedwalk("4n3es(nw/se)uw(say open sesame)dFLW".chars(), "filler")
                .expect("evaluation error");
        assert_eq!(
            speedwalk,
            "north\r\nnorth\r\nnorth\r\nnorth\r\neast\r\neast\r\neast\r\nsouth\r\nnw\r\nup\r\nwest\r\nsay open sesame\r\ndown\r\nfiller\r\nlock west\r\n"
        );
    }
}
