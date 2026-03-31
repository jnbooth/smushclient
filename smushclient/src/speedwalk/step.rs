use std::fmt;

fn write_repeatedly<T>(f: &mut fmt::Formatter, step: T, count: Option<u8>) -> fmt::Result
where
    T: fmt::Display,
{
    for _ in 0..count.unwrap_or(1) {
        write!(f, "{step}\r\n")?;
    }
    Ok(())
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Step<'a> {
    Filler(&'a str, Option<u8>),
    Interact(Interaction, Direction<'a>),
    Move(Direction<'a>, Option<u8>),
}

impl<'a> Step<'a> {
    pub fn reverse(self) -> Self {
        match self {
            Self::Filler(filler, count) => Self::Filler(filler, count),
            Self::Interact(act, dir) => Self::Interact(act, dir.reverse()),
            Self::Move(dir, count) => Self::Move(dir.reverse(), count),
        }
    }

    pub fn with_filler(self, filler: &'a str) -> Self {
        match self {
            Self::Filler(_, count) => Self::Filler(filler, count),
            _ => self,
        }
    }
}

impl fmt::Display for Step<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if f.width().is_some() || f.align().is_some() {
            #[allow(clippy::recursive_format_impl)]
            return f.pad(&self.to_string());
        }
        if f.alternate() {
            // encode
            match *self {
                Step::Filler(_, None) => write!(f, "F"),
                Step::Filler(_, Some(count)) => write!(f, "{count}F"),
                Step::Interact(act, dir) => write!(f, "{act:#}{dir:#}"),
                Step::Move(dir, None) => write!(f, "{dir:#}"),
                Step::Move(dir, Some(count)) => write!(f, "{count}{dir:#}"),
            }
        } else {
            // decode
            match *self {
                Self::Filler(filler, count) => write_repeatedly(f, filler, count),
                Self::Interact(act, dir) => write!(f, "{act} {dir}\r\n"),
                Self::Move(dir, count) => write_repeatedly(f, dir, count),
            }
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Interaction {
    Close,
    Open,
    Lock,
    Unlock,
}

impl fmt::Display for Interaction {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if f.alternate() {
            // encode
            match *self {
                Interaction::Close => f.pad("C"),
                Interaction::Open => f.pad("O"),
                Interaction::Lock => f.pad("L"),
                Interaction::Unlock => f.pad("K"),
            }
        } else {
            // decode
            match *self {
                Self::Close => f.pad("close"),
                Self::Open => f.pad("open"),
                Self::Lock => f.pad("lock"),
                Self::Unlock => f.pad("unlock"),
            }
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum Direction<'a> {
    North,
    South,
    East,
    West,
    Up,
    Down,
    Custom { forward: &'a str, backward: &'a str },
}

impl Direction<'_> {
    pub fn reverse(self) -> Self {
        match self {
            Self::North => Self::South,
            Self::South => Self::North,
            Self::East => Self::West,
            Self::West => Self::East,
            Self::Up => Self::Down,
            Self::Down => Self::Up,
            Self::Custom { forward, backward } if !backward.is_empty() => Self::Custom {
                forward: backward,
                backward: forward,
            },
            Self::Custom { forward, backward } => Self::Custom { forward, backward },
        }
    }
}

impl fmt::Display for Direction<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if f.alternate() {
            // encode
            match *self {
                Direction::North => f.pad("N"),
                Direction::South => f.pad("S"),
                Direction::East => f.pad("E"),
                Direction::West => f.pad("W"),
                Direction::Up => f.pad("U"),
                Direction::Down => f.pad("D"),
                Direction::Custom { forward, backward } => {
                    let args = if backward.is_empty() {
                        format_args!("({forward})")
                    } else {
                        format_args!("({forward}/{backward})")
                    };
                    if f.width().is_none() && f.align().is_none() {
                        f.write_fmt(args)
                    } else {
                        f.pad(&fmt::format(args))
                    }
                }
            }
        } else {
            // decode
            match *self {
                Self::North => f.pad("north"),
                Self::South => f.pad("south"),
                Self::East => f.pad("east"),
                Self::West => f.pad("west"),
                Self::Up => f.pad("up"),
                Self::Down => f.pad("down"),
                Self::Custom { forward, .. } => f.pad(forward),
            }
        }
    }
}
