use bytemuck::{Pod, Zeroable};

#[cfg(target_endian = "little")]
#[repr(C)]
#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, Zeroable, Pod)]
pub struct Pixel {
    pub blue: u8,
    pub green: u8,
    pub red: u8,
    pub alpha: u8,
}

#[cfg(target_endian = "big")]
#[repr(C)]
#[derive(Copy, Clone, Debug, Default, PartialEq, Eq, Zeroable, Pod)]
pub struct Pixel {
    pub alpha: u8,
    pub red: u8,
    pub green: u8,
    pub blue: u8,
}

impl Pixel {
    pub const fn transparent() -> Self {
        Self {
            blue: 0,
            green: 0,
            red: 0,
            alpha: 0,
        }
    }
}
