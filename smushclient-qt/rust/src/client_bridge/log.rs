use std::io;

use cxx_qt::CxxQtType;
use cxx_qt_lib::QString;
use smushclient::SmushClient;
use smushclient::world::LogMode;

use crate::ffi::{self, ApiCode, BytesView, StringView};

impl ffi::SmushClient {
    pub fn close_log(&self) -> ApiCode {
        let client = &self.rust().client;
        if !client.is_log_open() {
            return ApiCode::LogFileNotOpen;
        }
        let _ = client.close_log();
        ApiCode::OK
    }

    pub fn flush_log(&self) -> ApiCode {
        self.try_log(SmushClient::flush_log)
    }

    pub fn is_log_open(&self) -> bool {
        self.rust().client.is_log_open()
    }

    pub fn log_input(&self, note: &QString) -> ApiCode {
        self.try_log(|client| client.log_input(&String::from(note)))
    }

    pub fn log_note(&self, note: StringView<'_>) -> ApiCode {
        self.try_log(|client| client.log_note(&note.to_string_lossy()))
    }

    pub fn open_log(&self, path: StringView, append: bool) -> ApiCode {
        let Ok(path) = path.to_str() else {
            return ApiCode::FileNotFound;
        };
        let mode = if append {
            LogMode::Append
        } else {
            LogMode::Overwrite
        };
        let client = &self.rust().client;
        if client.is_log_open() {
            ApiCode::LogFileAlreadyOpen
        } else if client.open_log(path.to_owned(), Some(mode)).is_ok() {
            ApiCode::OK
        } else {
            ApiCode::CouldNotOpenFile
        }
    }

    pub fn try_close_log(&self) -> io::Result<()> {
        self.rust().client.close_log()
    }

    pub fn try_open_log(&self) -> io::Result<()> {
        self.rust().client.open_log(String::new(), None)
    }

    pub fn write_to_log(&self, note: BytesView<'_>) -> ApiCode {
        self.try_log(|client| client.write_to_log(note.as_slice()))
    }

    fn try_log<F>(&self, f: F) -> ApiCode
    where
        F: FnOnce(&SmushClient) -> io::Result<()>,
    {
        let client = &self.rust().client;
        if !client.is_log_open() {
            ApiCode::LogFileNotOpen
        } else if f(client).is_ok() {
            ApiCode::OK
        } else {
            ApiCode::LogFileBadWrite
        }
    }
}
