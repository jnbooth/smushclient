#[cxx::bridge]
mod ffi {
    #[repr(i32)]
    #[namespace = "rust::smushclientqtlib1"]
    enum QTextCursorMoveMode {
        MoveAnchor,
        KeepAnchor,
    }

    #[repr(i32)]
    #[namespace = "rust::smushclientqtlib1"]
    enum QTextCursorMoveOperation {
        NoMove,

        Start,
        Up,
        StartOfLine,
        StartOfBlock,
        StartOfWord,
        PreviousBlock,
        PreviousCharacter,
        PreviousWord,
        Left,
        WordLeft,

        End,
        Down,
        EndOfLine,
        EndOfWord,
        EndOfBlock,
        NextBlock,
        NextCharacter,
        NextWord,
        Right,
        WordRight,

        NextCell,
        PreviousCell,
        NextRow,
        PreviousRow,
    }

    #[namespace = "rust::smushclientqtlib1"]
    extern "C++" {
        include!("smushclient-qt-lib/qtextcursor.h");
        type QTextCursorMoveMode;
        type QTextCursorMoveOperation;
    }
}

pub use ffi::QTextCursorMoveOperation;
