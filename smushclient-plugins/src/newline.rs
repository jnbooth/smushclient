use std::borrow::Cow;

pub trait CharWithNewline: Eq {
    const CR: Self;
    const LF: Self;
}

impl CharWithNewline for u8 {
    const CR: Self = b'\r';
    const LF: Self = b'\n';
}

impl CharWithNewline for u16 {
    const CR: Self = b'\r' as u16;
    const LF: Self = b'\n' as u16;
}

impl CharWithNewline for char {
    const CR: Self = '\r';
    const LF: Self = '\n';
}

pub struct CrLfIter<I> {
    inner: I,
    had_cr: bool,
    need_lf: bool,
}

impl<I> Iterator for CrLfIter<I>
where
    I: Iterator,
    I::Item: CharWithNewline,
{
    type Item = I::Item;

    #[inline]
    fn next(&mut self) -> Option<Self::Item> {
        if self.need_lf {
            self.need_lf = false;
            return Some(I::Item::LF);
        }
        let next = self.inner.next()?;
        self.need_lf = next == I::Item::LF && !self.had_cr;
        self.had_cr = next == I::Item::CR;
        if self.need_lf {
            return Some(I::Item::CR);
        }
        Some(next)
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.inner.size_hint().0, None)
    }
}

#[inline]
pub fn ensure_crlf<I>(iter: I) -> CrLfIter<I> {
    CrLfIter {
        inner: iter,
        had_cr: false,
        need_lf: false,
    }
}

#[inline]
pub fn replace_crlf(s: &str) -> String {
    let mut last_char = '\0';
    s.replace(
        |c| {
            let should_replace = c == '\n' && last_char != '\r';
            last_char = c;
            should_replace
        },
        "\r\n",
    )
}

pub(crate) fn ensure_send_crlf(s: Cow<'_, str>) -> String {
    if !s.as_bytes().contains(&b'\n') {
        return s.into_owned();
    }
    replace_crlf(&s)
}

#[cfg(test)]
mod tests {
    #[test]
    fn ensure_crlf() {
        let s = "test\r\r\n\nof\r \nthis\n";
        assert_eq!(
            super::ensure_crlf(s.chars()).collect::<String>(),
            "test\r\r\n\r\nof\r \r\nthis\r\n"
        );
    }

    #[test]
    fn replace_crlf() {
        let s = "test\r\r\n\nof\r \nthis\n";
        assert_eq!(super::replace_crlf(s), "test\r\r\n\r\nof\r \r\nthis\r\n");
    }
}
