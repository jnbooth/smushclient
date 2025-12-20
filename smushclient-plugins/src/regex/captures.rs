use std::iter::FusedIterator;
use std::ops::Range;

pub struct Match<'s> {
    subject: &'s str,
    start: usize,
    end: usize,
}

impl<'s> Match<'s> {
    fn new(subject: &'s str, matched: pcre2::bytes::Match) -> Option<Self> {
        let start = matched.start();
        let end = matched.end();
        Some(Self {
            subject: subject.get(start..end)?,
            start,
            end,
        })
    }

    pub const fn start(&self) -> usize {
        self.start
    }

    pub const fn end(&self) -> usize {
        self.end
    }

    pub const fn as_bytes(&self) -> &'s [u8] {
        self.subject.as_bytes()
    }

    pub const fn as_str(&self) -> &'s str {
        self.subject
    }
}

impl AsRef<[u8]> for Match<'_> {
    fn as_ref(&self) -> &[u8] {
        self.as_bytes()
    }
}

impl AsRef<str> for Match<'_> {
    fn as_ref(&self) -> &str {
        self.as_str()
    }
}

pub struct Captures<'s> {
    subject: &'s str,
    inner: pcre2::bytes::Captures<'s>,
}

impl<'s> Captures<'s> {
    pub fn get(&self, i: usize) -> Option<Match<'s>> {
        Match::new(self.subject, self.inner.get(i)?)
    }

    pub fn name(&self, name: &str) -> Option<Match<'s>> {
        Match::new(self.subject, self.inner.name(name)?)
    }

    pub fn iter(&self) -> CapturesIter<'_, 's> {
        self.into_iter()
    }

    #[allow(clippy::len_without_is_empty)]
    pub fn len(&self) -> usize {
        self.inner.len()
    }
}

impl<'r, 's> IntoIterator for &'r Captures<'s> {
    type Item = &'s str;

    type IntoIter = CapturesIter<'r, 's>;

    fn into_iter(self) -> Self::IntoIter {
        CapturesIter {
            inner: self,
            range: 1..self.len(),
        }
    }
}

pub struct CaptureMatches<'r, 's> {
    pub(super) subject: &'s str,
    pub(super) inner: pcre2::bytes::CaptureMatches<'r, 's>,
}

impl<'s> Iterator for CaptureMatches<'_, 's> {
    type Item = Result<Captures<'s>, pcre2::Error>;

    fn next(&mut self) -> Option<Self::Item> {
        match self.inner.next() {
            None => None,
            Some(Err(e)) => Some(Err(e)),
            Some(Ok(captures)) => Some(Ok(Captures {
                subject: self.subject,
                inner: captures,
            })),
        }
    }
}

pub struct CapturesIter<'r, 's> {
    inner: &'r Captures<'s>,
    range: Range<usize>,
}

impl<'s> Iterator for CapturesIter<'_, 's> {
    type Item = &'s str;

    fn next(&mut self) -> Option<Self::Item> {
        Some(match self.inner.get(self.range.next()?) {
            Some(capture) => capture.as_str(),
            None => "",
        })
    }
}

impl FusedIterator for CapturesIter<'_, '_> {}

impl DoubleEndedIterator for CapturesIter<'_, '_> {
    fn next_back(&mut self) -> Option<Self::Item> {
        Some(match self.inner.get(self.range.next_back()?) {
            Some(capture) => capture.as_str(),
            None => "",
        })
    }
}
