#[cfg(target_endian = "little")]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum ColorChannel {
    Blue,
    Green,
    Red,
    Alpha,
}

#[cfg(target_endian = "big")]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum ColorChannel {
    Blue = 3,
    Green = 2,
    Red = 1,
    Alpha = 0,
}
