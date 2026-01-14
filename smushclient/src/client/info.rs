use std::cell::{Cell, RefCell};

use mud_transformer::Bytes;

#[derive(Debug, Default)]
pub(super) struct ClientInfo {
    pub last_line_with_iac_ga: Cell<i64>,
    pub last_subnegotiation: RefCell<Bytes>,
    pub lines_displayed: Cell<i64>,
    pub lines_received: Cell<i64>,
    pub packets_received: Cell<i64>,
    pub simulating: Cell<bool>,
    pub variables_dirty: Cell<bool>,
}

impl ClientInfo {
    pub fn reset(&self) {
        self.last_line_with_iac_ga.set(0);
        self.last_subnegotiation.borrow_mut().clear();
        self.lines_displayed.set(0);
        self.lines_received.set(0);
        self.simulating.set(false);
        self.variables_dirty.set(false);
    }
}
