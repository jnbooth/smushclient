use std::pin::Pin;

use cxx_qt::CxxQtType;
use cxx_qt_io::QAbstractSocket;
use cxx_qt_lib::QString;

use crate::ffi;

impl ffi::SmushClient {
    pub fn bytes_received(&self) -> u64 {
        self.rust().client.bytes_received()
    }

    pub fn connect_to_host(&self, socket: Pin<&mut QAbstractSocket>) {
        self.rust().connect_to_host(socket);
    }

    pub fn flush(&self, doc: Pin<&mut ffi::Document>) {
        self.rust().flush(doc);
    }

    pub fn handle_connect(&self, socket: Pin<&mut ffi::QAbstractSocket>) -> QString {
        self.rust().handle_connect(socket)
    }

    pub fn handle_disconnect(&self) {
        self.rust().handle_disconnect();
    }

    pub fn has_output(&self) -> bool {
        self.rust().client.has_output()
    }

    pub fn read(
        &self,
        device: Pin<&mut ffi::QAbstractSocket>,
        doc: Pin<&mut ffi::Document>,
    ) -> i64 {
        self.rust().read(device, doc)
    }

    pub fn reset_mxp(&self) {
        self.rust().client.reset_mxp();
    }
}
