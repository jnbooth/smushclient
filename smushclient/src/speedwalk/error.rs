use std::fmt;
use std::str::Utf8Error;

pub(super) const COUNTER_LIMIT: u8 = 99;

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Error {
    UnterminatedComment,
    CounterExceedsLimit,
    CounterWithoutAction,
    CounterWithColk,
    ColkWithoutDirection,
    UnterminatedParenthesis,
    InvalidDirection(char),
    InvalidUtf8(Utf8Error),
}

impl fmt::Display for Error {
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
            Self::InvalidUtf8(error) => error.fmt(f),
        }
    }
}

impl std::error::Error for Error {}

impl From<Utf8Error> for Error {
    fn from(value: Utf8Error) -> Self {
        Self::InvalidUtf8(value)
    }
}
