use std::ptr;

use mud_transformer::mxp::{Heading, Link, NamedColorIter, RgbColor, SendTo};
use mud_transformer::{TextFragment, TextStyle};

use crate::ffi;

#[repr(transparent)]
pub struct RustMxpLink {
    inner: Link,
}

impl RustMxpLink {
    fn cast(link: &Link) -> &Self {
        // SAFETY: #[repr(transparent)]
        unsafe { &*ptr::from_ref::<Link>(link).cast::<Self>() }
    }

    pub fn action(&self) -> &str {
        &self.inner.action
    }

    pub fn hint(&self) -> Option<&str> {
        self.inner.hint.as_deref()
    }

    pub fn prompts(&self) -> Vec<String> {
        self.inner.prompts.clone()
    }

    pub fn sendto(&self) -> SendTo {
        self.inner.sendto
    }
}

#[repr(transparent)]
pub struct RustTextFragment {
    inner: TextFragment,
}

impl From<TextFragment> for RustTextFragment {
    fn from(inner: TextFragment) -> Self {
        Self { inner }
    }
}

macro_rules! flag_method {
    ($n:ident, $v:expr) => {
        #[inline(always)]
        pub fn $n(&self) -> bool {
            self.inner.flags.contains($v)
        }
    };
}

impl RustTextFragment {
    #[inline(always)]
    pub fn text(&self) -> &str {
        &self.inner.text
    }

    #[inline(always)]
    pub fn foreground(&self) -> RgbColor {
        self.inner.foreground
    }

    #[inline(always)]
    pub fn background(&self) -> RgbColor {
        self.inner.background
    }

    pub fn link(&self) -> Option<&RustMxpLink> {
        self.inner.action.as_ref().map(RustMxpLink::cast)
    }

    pub fn heading(&self) -> Option<Heading> {
        self.inner.heading
    }

    flag_method!(is_blink, TextStyle::Blink);
    flag_method!(is_bold, TextStyle::Bold);
    flag_method!(is_highlight, TextStyle::Highlight);
    flag_method!(is_italic, TextStyle::Italic);
    flag_method!(is_strikeout, TextStyle::Strikeout);
    flag_method!(is_underline, TextStyle::Underline);
}

#[repr(transparent)]
pub struct RustNamedColorIter {
    inner: NamedColorIter,
}

impl Default for RustNamedColorIter {
    fn default() -> Self {
        Self::new()
    }
}

impl RustNamedColorIter {
    pub fn new() -> Self {
        Self {
            inner: RgbColor::iter_named(),
        }
    }

    pub fn next(&mut self) -> Option<ffi::RgbColor> {
        self.inner.next().map(|(_k, color)| color.into())
    }
}
