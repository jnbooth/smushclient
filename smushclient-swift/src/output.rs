use mud_transformer::mxp::{self, SendTo, WorldColor};
use mud_transformer::{TextFragment, TextStyle};

#[repr(transparent)]
pub struct RustMxpLink {
    inner: mxp::Link,
}

impl RustMxpLink {
    fn cast(link: &mxp::Link) -> &Self {
        // SAFETY: #[repr(transparent)]
        unsafe { &*(link as *const mxp::Link as *const Self) }
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
    pub fn foreground(&self) -> WorldColor {
        self.inner.foreground
    }

    #[inline(always)]
    pub fn background(&self) -> WorldColor {
        self.inner.background
    }

    #[inline]
    pub fn link(&self) -> Option<&RustMxpLink> {
        self.inner
            .action
            .as_ref()
            .map(|action| RustMxpLink::cast(action))
    }

    flag_method!(is_blink, TextStyle::Blink);
    flag_method!(is_bold, TextStyle::Bold);
    flag_method!(is_highlight, TextStyle::Highlight);
    flag_method!(is_inverse, TextStyle::Inverse);
    flag_method!(is_italic, TextStyle::Italic);
    flag_method!(is_strikeout, TextStyle::Strikeout);
    flag_method!(is_underline, TextStyle::Underline);
}