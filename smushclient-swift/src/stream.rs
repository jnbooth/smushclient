use std::{mem, vec};

use crate::ffi;
use smushclient::AliasOutcome;

pub struct RustAliasOutcome {
    outcome: AliasOutcome,
    requests: Vec<ffi::SendRequest>,
}

impl RustAliasOutcome {
    pub fn new(outcome: AliasOutcome, requests: Vec<ffi::SendRequest>) -> Self {
        Self { outcome, requests }
    }

    pub fn stream(&mut self) -> RustSendStream {
        RustSendStream {
            inner: mem::take(&mut self.requests).into_iter(),
        }
    }

    pub fn should_remember(&self) -> bool {
        self.outcome.remember
    }

    pub fn should_send(&self) -> bool {
        self.outcome.send
    }
}

#[repr(transparent)]
pub struct RustSendStream {
    inner: vec::IntoIter<ffi::SendRequest>,
}

impl RustSendStream {
    #[inline(always)]
    pub fn next(&mut self) -> Option<ffi::SendRequest> {
        self.inner.next()
    }

    #[inline(always)]
    pub fn count(&self) -> usize {
        self.inner.len()
    }
}

#[repr(transparent)]
pub struct RustOutputStream {
    inner: vec::IntoIter<ffi::OutputFragment>,
}

impl RustOutputStream {
    pub const fn new(output: vec::IntoIter<ffi::OutputFragment>) -> Self {
        Self { inner: output }
    }

    #[inline(always)]
    pub fn next(&mut self) -> Option<ffi::OutputFragment> {
        self.inner.next()
    }

    #[inline(always)]
    pub fn count(&self) -> usize {
        self.inner.len()
    }
}
