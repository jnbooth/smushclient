use std::mem::MaybeUninit;

/// # Safety
///
/// `constructor` must initialize the passed pointer in-place.
pub unsafe fn new_in_place<T, F>(constructor: F) -> T
where
    F: FnOnce(*mut T),
{
    let mut uninit = MaybeUninit::uninit();
    constructor(uninit.as_mut_ptr());
    // SAFETY: `constructor` initializes the passed pointer in-place.
    unsafe { uninit.assume_init() }
}
