use std::cell::{Cell, RefCell};

use mud_transformer::Bytes;

#[derive(Debug, Default)]
pub struct ClientInfo {
    pub bytes_received: Cell<u64>,
    pub bytes_received_compressed: Cell<u64>,
    pub bytes_received_uncompressed: Cell<u64>,
    pub last_capture: Cell<usize>,
    pub last_line_with_iac_ga: Cell<u64>,
    pub last_match: RefCell<String>,
    pub last_subnegotiation: RefCell<Bytes>,
    pub lines_displayed: Cell<u64>,
    pub lines_received: Cell<u64>,
    pub packets_received: Cell<u64>,
    pub simulating: Cell<bool>,
}

impl ClientInfo {
    pub fn reset(&self) {
        self.bytes_received.set(0);
        self.bytes_received_compressed.set(0);
        self.bytes_received_uncompressed.set(0);
        self.last_capture.set(0);
        self.last_line_with_iac_ga.set(0);
        self.last_match.borrow_mut().clear();
        self.last_subnegotiation.borrow_mut().clear();
        self.lines_displayed.set(0);
        self.lines_received.set(0);
        self.packets_received.set(0);
        self.simulating.set(false);
    }
}
