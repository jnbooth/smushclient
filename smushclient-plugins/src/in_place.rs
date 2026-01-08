use std::borrow::Cow;
use std::num::NonZero;

pub trait InPlace<T> {
    fn in_place(self) -> T;
}
impl<T: Sized + Copy> InPlace<T> for T {
    fn in_place(self) -> T {
        self
    }
}
impl<'a> InPlace<Cow<'a, str>> for &'a String {
    fn in_place(self) -> Cow<'a, str> {
        Cow::Borrowed(self.as_str())
    }
}
impl InPlace<String> for Cow<'_, str> {
    fn in_place(self) -> String {
        self.into_owned()
    }
}
impl InPlace<String> for Vec<Cow<'_, str>> {
    fn in_place(self) -> String {
        let mut lines = self.into_iter();
        let mut s = match lines.next() {
            Some(s) => s.into_owned(),
            None => return String::new(),
        };
        for line in lines {
            s.push('\n');
            s.push_str(&line);
        }
        s
    }
}
impl<'a> InPlace<Vec<Cow<'a, str>>> for &'a String {
    fn in_place(self) -> Vec<Cow<'a, str>> {
        vec![Cow::Borrowed(self)]
    }
}

impl InPlace<Option<NonZero<u8>>> for u8 {
    fn in_place(self) -> Option<NonZero<u8>> {
        NonZero::new(self)
    }
}

impl InPlace<u8> for Option<NonZero<u8>> {
    fn in_place(self) -> u8 {
        match self {
            Some(n) => n.get(),
            None => 0,
        }
    }
}

macro_rules! in_place {
    ($from:expr, $name:ident {
        $($field:ident $(: $val:expr)?,)* $(,)?
        $(..$flat:ident),* $(,)*
    }) => {
        $name {
            $($field $(: $val)?,)*
            $($flat: $from.$flat.in_place()),*
        }
    }
}
pub(crate) use in_place;
