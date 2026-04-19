use std::borrow::Cow;

pub(crate) struct Clipboard {
    base: LazyClipboard,
}

impl Default for Clipboard {
    fn default() -> Self {
        Self::new()
    }
}

impl Clipboard {
    pub const fn new() -> Self {
        Self {
            base: LazyClipboard::Uninitialized,
        }
    }

    pub fn set_text<'a, T: Into<Cow<'a, str>>>(&mut self, text: T) -> Result<(), arboard::Error> {
        let Some(clipboard) = self.clipboard_mut()? else {
            return Ok(());
        };
        clipboard.set_text(text)
    }

    fn clipboard_mut(&mut self) -> Result<Option<&mut arboard::Clipboard>, arboard::Error> {
        self.base.get_mut()
    }
}

enum LazyClipboard {
    Uninitialized,
    Initialized(arboard::Clipboard),
    Unsupported,
}

impl LazyClipboard {
    pub fn get_mut(&mut self) -> Result<Option<&mut arboard::Clipboard>, arboard::Error> {
        match self {
            Self::Initialized(clipboard) => return Ok(Some(clipboard)),
            Self::Unsupported => return Ok(None),
            Self::Uninitialized => (),
        }
        let clipboard = arboard::Clipboard::new().inspect_err(|_| *self = Self::Unsupported)?;
        *self = Self::Initialized(clipboard);
        match self {
            Self::Initialized(clipboard) => Ok(Some(clipboard)),
            _ => unreachable!(),
        }
    }
}
