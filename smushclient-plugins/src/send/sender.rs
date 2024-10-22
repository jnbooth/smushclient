use std::cmp;
use std::error::Error;
use std::fmt::{self, Display, Formatter};
use std::ops::{Deref, DerefMut};
use std::sync::atomic::{AtomicBool, Ordering};

use serde::{Deserialize, Serialize};

use super::send_to::SendTarget;

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Deserialize, Serialize)]
#[serde(default)]
pub struct Sender {
    // Note: these two fields are at the top for Ord-deriving purposes.
    pub group: String,
    pub label: String,
    pub send_to: SendTarget,
    pub script: String,
    pub variable: String,
    pub text: String,

    pub enabled: bool,
    pub one_shot: bool,
    pub temporary: bool,
    pub omit_from_output: bool,
    pub omit_from_log: bool,

    #[serde(skip)]
    pub lock: SenderLock,
}

impl Default for Sender {
    fn default() -> Self {
        Self {
            text: String::new(),
            send_to: SendTarget::World,
            label: String::new(),
            script: String::new(),
            group: String::new(),
            variable: String::new(),
            enabled: true,
            one_shot: false,
            temporary: false,
            omit_from_output: false,
            omit_from_log: false,
            lock: SenderLock(AtomicBool::new(false)),
        }
    }
}

impl Sender {
    pub fn lock(&self) -> bool {
        !self.lock.swap(true, Ordering::Relaxed)
    }

    pub fn unlock(&self) {
        self.lock.store(false, Ordering::Relaxed);
    }

    pub fn is_locked(&self) -> bool {
        self.lock.load(Ordering::Relaxed)
    }

    /// This function should be called before modifying any of the Sender's string fields, or before
    /// dropping a Sender.
    pub fn try_unlock(&self) -> Result<(), SenderLockError> {
        if self.lock.load(Ordering::Relaxed) {
            Err(SenderLockError)
        } else {
            Ok(())
        }
    }
}

#[derive(Debug, Default)]
#[repr(transparent)]
pub struct SenderLock(AtomicBool);

impl SenderLock {
    pub const fn new(value: bool) -> Self {
        Self(AtomicBool::new(value))
    }
}

impl From<bool> for SenderLock {
    fn from(value: bool) -> Self {
        Self(AtomicBool::new(value))
    }
}

impl From<SenderLock> for bool {
    fn from(value: SenderLock) -> Self {
        value.load(Ordering::Relaxed)
    }
}

impl Deref for SenderLock {
    type Target = AtomicBool;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for SenderLock {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl Clone for SenderLock {
    fn clone(&self) -> Self {
        Self(AtomicBool::new(false))
    }
}

impl PartialEq for SenderLock {
    fn eq(&self, _: &Self) -> bool {
        true
    }
}

impl Eq for SenderLock {}

impl PartialOrd for SenderLock {
    fn partial_cmp(&self, other: &Self) -> Option<cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for SenderLock {
    fn cmp(&self, _: &Self) -> cmp::Ordering {
        cmp::Ordering::Equal
    }
}

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct SenderLockError;

impl Display for SenderLockError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        f.write_str("concurrent sender modification")
    }
}

impl Error for SenderLockError {}
