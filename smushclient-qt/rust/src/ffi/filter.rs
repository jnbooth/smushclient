#![allow(clippy::cast_sign_loss)]
use smushclient_graphics::filter::{
    self, average, grayscale_linear, grayscale_perceptual, mono_noise, noise,
};
use smushclient_graphics::{ColorChannel, Directions};

#[cxx::bridge]
mod ffi {
    #[namespace = "ffi::filter"]
    #[repr(u8)]
    enum ColorChannel {
        Blue,
        Green,
        Red,
        All,
    }

    #[namespace = "ffi::filter"]
    #[repr(u8)]
    enum Directions {
        Both,
        Horizontal,
        Vertical,
    }

    #[namespace = "ffi::filter"]
    extern "Rust" {
        fn noise(data: &mut [u32], threshold: f64);
        fn mono_noise(data: &mut [u32], threshold: f64);
        fn blur(data: &mut [u32], width: i32, directions: Directions);
        fn sharpen(data: &mut [u32], width: i32, directions: Directions);
        fn edge_detect(data: &mut [u32], width: i32, directions: Directions);
        fn emboss(data: &mut [u32], width: i32, directions: Directions);
        fn brightness_add(data: &mut [u32], add: i32, channel: ColorChannel);
        fn contrast(data: &mut [u32], mult: f64, channel: ColorChannel);
        fn gamma(data: &mut [u32], exp: f64, channel: ColorChannel);
        fn grayscale_linear(data: &mut [u32]);
        fn grayscale_perceptual(data: &mut [u32]);
        fn brightness_mult(data: &mut [u32], mult: f64, channel: ColorChannel);
        fn lesser_blur(data: &mut [u32], width: i32, directions: Directions);
        fn minor_blur(data: &mut [u32], width: i32, directions: Directions);
        fn average(data: &mut [u32]);
    }
}

impl From<ffi::ColorChannel> for Option<ColorChannel> {
    fn from(value: ffi::ColorChannel) -> Self {
        match value {
            ffi::ColorChannel::Blue => Some(ColorChannel::Blue),
            ffi::ColorChannel::Green => Some(ColorChannel::Green),
            ffi::ColorChannel::Red => Some(ColorChannel::Red),
            _ => None,
        }
    }
}

impl From<ffi::Directions> for Directions {
    fn from(value: ffi::Directions) -> Self {
        match value {
            ffi::Directions::Horizontal => Directions::Horizontal,
            ffi::Directions::Vertical => Directions::Vertical,
            _ => Directions::Both,
        }
    }
}

fn blur(data: &mut [u32], width: i32, directions: ffi::Directions) {
    filter::blur(data, width as usize, directions.into());
}

fn sharpen(data: &mut [u32], width: i32, directions: ffi::Directions) {
    filter::sharpen(data, width as usize, directions.into());
}

fn edge_detect(data: &mut [u32], width: i32, directions: ffi::Directions) {
    filter::edge_detect(data, width as usize, directions.into());
}

fn emboss(data: &mut [u32], width: i32, directions: ffi::Directions) {
    filter::emboss(data, width as usize, directions.into());
}

fn brightness_add(data: &mut [u32], add: i32, channel: ffi::ColorChannel) {
    filter::brightness_add(data, add, channel.into());
}

fn contrast(data: &mut [u32], mult: f64, channel: ffi::ColorChannel) {
    filter::contrast(data, mult, channel.into());
}

fn gamma(data: &mut [u32], exp: f64, channel: ffi::ColorChannel) {
    filter::gamma(data, exp, channel.into());
}

fn brightness_mult(data: &mut [u32], mult: f64, channel: ffi::ColorChannel) {
    filter::brightness_mult(data, mult, channel.into());
}

fn lesser_blur(data: &mut [u32], width: i32, directions: ffi::Directions) {
    filter::lesser_blur(data, width as usize, directions.into());
}

fn minor_blur(data: &mut [u32], width: i32, directions: ffi::Directions) {
    filter::minor_blur(data, width as usize, directions.into());
}
