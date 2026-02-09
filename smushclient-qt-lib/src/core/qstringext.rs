use std::ptr;

use cxx_qt_lib::{CaseSensitivity, QString, QStringList, SplitBehaviorFlags};

use crate::QChar;

#[cxx::bridge]
mod ffi {
    extern "C++" {
        include!("cxx-qt-lib/qstring.h");
        type QString = cxx_qt_lib::QString;
        include!("cxx-qt-lib/qstringlist.h");
        type QStringList = cxx_qt_lib::QStringList;
    }

    #[namespace = "Qt"]
    extern "C++" {
        include!("cxx-qt-lib/qt.h");
        type CaseSensitivity = cxx_qt_lib::CaseSensitivity;
        type SplitBehaviorFlags = cxx_qt_lib::SplitBehaviorFlags;
    }

    extern "C++" {
        include!("smushclient-qt-lib/qchar.h");
        type QChar = crate::QChar;
    }

    #[namespace = "rust::smushclientqtlib1::qstring"]
    unsafe extern "C++" {
        include!("smushclient-qt-lib/qstringext.h");
        fn append(s: &mut QString, ch: QChar) -> &mut QString;

        /// # Safety
        ///
        /// `position` must be a valid index position in the string (i.e. 0 <= position < s.size()).
        unsafe fn at(s: &QString, position: isize) -> QChar;

        fn split(
            s: &QString,
            delim: QChar,
            behavior: SplitBehaviorFlags,
            cs: CaseSensitivity,
        ) -> QStringList;
    }
}

pub trait QStringExt {
    fn append_char(&mut self, ch: QChar) -> &mut Self;
    fn chars(&self) -> &[QChar];
    fn at(&self, position: isize) -> Option<QChar>;
    fn back(&self) -> Option<QChar>;
    fn front(&self) -> Option<QChar>;
    fn split_char(
        &self,
        delim: QChar,
        behavior: SplitBehaviorFlags,
        cs: CaseSensitivity,
    ) -> QStringList;
}

impl QStringExt for QString {
    fn append_char(&mut self, ch: QChar) -> &mut Self {
        ffi::append(self, ch)
    }

    fn chars(&self) -> &[QChar] {
        // SAFETY: QChar is u16
        unsafe { &*(ptr::from_ref(self.as_slice()) as *const [QChar]) }
    }

    fn at(&self, position: isize) -> Option<QChar> {
        if position >= 0 && position < self.len() {
            // SAFETY: 0 <= position < s.size()
            Some(unsafe { ffi::at(self, position) })
        } else {
            None
        }
    }

    fn back(&self) -> Option<QChar> {
        let position = self.len() - 1;
        if position >= 0 {
            // SAFETY: 0 <= position < s.size()
            Some(unsafe { ffi::at(self, position) })
        } else {
            None
        }
    }

    fn front(&self) -> Option<QChar> {
        if self.is_empty() {
            None
        } else {
            // SAFETY: 0 <= position < s.size()
            Some(unsafe { ffi::at(self, 0) })
        }
    }

    fn split_char(
        &self,
        delim: QChar,
        behavior: SplitBehaviorFlags,
        cs: CaseSensitivity,
    ) -> QStringList {
        ffi::split(self, delim, behavior, cs)
    }
}
