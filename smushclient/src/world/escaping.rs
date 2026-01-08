use std::borrow::Cow;
use std::fmt;
use std::io::{self, Write};

use chrono::{DateTime, Local, TimeZone};

use super::World;

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Escaped<S = String> {
    message: S,
    has_chrono: bool,
}

impl<S> Escaped<S> {
    pub fn create<'a>(message: &'a str, world: &World) -> Self
    where
        S: From<&'a str> + From<String>,
    {
        // Reduce monomorphization
        fn inner<'a>(message: &'a str, world: &World) -> Escaped<Cow<'a, str>> {
            if message.len() <= 1 {
                return Escaped {
                    message: message.into(),
                    has_chrono: false,
                };
            }
            let mut has_world = false;
            let mut has_player = false;
            let mut has_chrono = false;
            let mut in_escape = false;
            for &byte in message.as_bytes() {
                if in_escape {
                    match byte {
                        b'N' => has_world = true,
                        b'P' => has_player = true,
                        _ => has_chrono = true,
                    }
                    in_escape = false;
                } else if byte == b'%' {
                    in_escape = true;
                }
            }
            let mut message = Cow::Borrowed(message);
            if has_world {
                message = message.replace("%N", &world.name).into();
            }
            if has_player {
                message = message.replace("%P", &world.player).into();
            }
            Escaped {
                message,
                has_chrono,
            }
        }

        let Escaped {
            message,
            has_chrono,
        } = inner(message, world);
        Self {
            message: match message {
                Cow::Borrowed(borrowed) => borrowed.into(),
                Cow::Owned(owned) => owned.into(),
            },
            has_chrono,
        }
    }
}

impl<S: AsRef<str>> Escaped<S> {
    pub fn is_empty(&self) -> bool {
        self.message.as_ref().is_empty()
    }
}

impl<S> Escaped<S> {
    pub fn with_datetime<Tz>(&self, datetime: DateTime<Tz>) -> EscapedChrono<'_, S, Tz>
    where
        Tz: TimeZone,
    {
        EscapedChrono {
            escaped: self,
            datetime,
        }
    }
}

impl<'a> Escaped<Cow<'a, str>> {
    pub fn borrow(message: &'a str, world: &World) -> Self {
        Self::create(message, world)
    }
}

impl Escaped<String> {
    pub fn new(message: &str, world: &World) -> Self {
        Self::create(message, world)
    }

    pub fn to_string(&self) -> Cow<'_, str> {
        if self.has_chrono {
            let mut result = String::new();
            #[allow(clippy::missing_panics_doc)]
            Local::now()
                .format(self.message.as_ref())
                .write_to(&mut result)
                .unwrap();
            Cow::Owned(result)
        } else {
            Cow::Borrowed(self.message.as_ref())
        }
    }
}

impl<S> fmt::Display for Escaped<S>
where
    S: AsRef<str>,
{
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let message = self.message.as_ref();
        if message.is_empty() {
            return Ok(());
        }
        if self.has_chrono {
            Local::now().format(message).fmt(f)
        } else {
            message.fmt(f)
        }
    }
}

pub struct EscapedChrono<'a, S, Tz: TimeZone> {
    escaped: &'a Escaped<S>,
    datetime: DateTime<Tz>,
}

impl<S, Tz> fmt::Display for EscapedChrono<'_, S, Tz>
where
    S: AsRef<str>,
    Tz: TimeZone,
    Tz::Offset: fmt::Display,
{
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let message = self.escaped.message.as_ref();
        if message.is_empty() {
            return Ok(());
        }
        if self.escaped.has_chrono {
            self.datetime.format(message).fmt(f)
        } else {
            message.fmt(f)
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct EscapedBrackets<S = String> {
    before: Escaped<S>,
    after: Escaped<S>,
}

impl<S> EscapedBrackets<S> {
    pub const fn new(before: Escaped<S>, after: Escaped<S>) -> Self {
        Self { before, after }
    }

    pub const fn before(&self) -> &Escaped<S> {
        &self.before
    }

    pub const fn after(&self) -> &Escaped<S> {
        &self.after
    }

    pub fn write<W: Write>(&self, mut writer: W, line: &[u8]) -> io::Result<()>
    where
        S: AsRef<str>,
    {
        let Self { before, after } = self;
        if !before.has_chrono && !after.has_chrono {
            writer.write_all(before.message.as_ref().as_bytes())?;
            writer.write_all(line)?;
            return writer.write_all(after.message.as_ref().as_bytes());
        }
        let now = Local::now();
        write!(writer, "{}", before.with_datetime(now))?;
        writer.write_all(line)?;
        write!(writer, "{}", after.with_datetime(now))
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct LogBrackets {
    pub file: EscapedBrackets<String>,
    pub output: EscapedBrackets<String>,
    pub input: EscapedBrackets<String>,
    pub notes: EscapedBrackets<String>,
}

impl From<&World> for LogBrackets {
    fn from(world: &World) -> Self {
        let mut file = EscapedBrackets::new(
            Escaped::new(&world.log_file_preamble, world),
            Escaped::new(&world.log_file_postamble, world),
        );
        let mut output = EscapedBrackets::new(
            Escaped::new(&world.log_line_preamble_output, world),
            Escaped::new(&world.log_line_postamble_output, world),
        );
        let mut input = EscapedBrackets::new(
            Escaped::new(&world.log_line_preamble_input, world),
            Escaped::new(&world.log_line_postamble_input, world),
        );
        let mut notes = EscapedBrackets::new(
            Escaped::new(&world.log_line_preamble_notes, world),
            Escaped::new(&world.log_line_postamble_notes, world),
        );
        if !file.before.message.is_empty() {
            file.before.message.push('\n');
        }
        if !file.after.message.is_empty() {
            file.after.message.push('\n');
        }
        output.after.message.push('\n');
        input.after.message.push('\n');
        notes.after.message.push('\n');
        Self {
            file,
            output,
            input,
            notes,
        }
    }
}
