use cxx_qt_lib::QColor;
use smushclient_graphics::filter::{
    self, average, dissolve, grayscale_linear, grayscale_perceptual, mask_premultiplied,
    mono_noise, noise,
};
use smushclient_graphics::{ColorChannel, Directions, Pixel};

use crate::convert::OutOfRangeError;

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

    extern "C++" {
        include!("cxx-qt-lib/qcolor.h");
        type QColor = cxx_qt_lib::QColor;
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

        fn color_to_alpha(data: &mut [u32], color: &QColor);
        fn dissolve(data: &mut [u32], opacity: f64);
        fn mask_premultiplied(data: &mut [u32], mask: &[u8], opacity: f64) -> bool;
    }
}

impl TryFrom<ffi::ColorChannel> for Option<ColorChannel> {
    type Error = OutOfRangeError;

    fn try_from(value: ffi::ColorChannel) -> Result<Self, Self::Error> {
        match value {
            ffi::ColorChannel::Blue => Ok(Some(ColorChannel::Blue)),
            ffi::ColorChannel::Green => Ok(Some(ColorChannel::Green)),
            ffi::ColorChannel::Red => Ok(Some(ColorChannel::Red)),
            ffi::ColorChannel::All => Ok(None),
            _ => Err(OutOfRangeError),
        }
    }
}

impl TryFrom<ffi::Directions> for Directions {
    type Error = OutOfRangeError;

    fn try_from(value: ffi::Directions) -> Result<Self, Self::Error> {
        match value {
            ffi::Directions::Both => Ok(Directions::Both),
            ffi::Directions::Horizontal => Ok(Directions::Horizontal),
            ffi::Directions::Vertical => Ok(Directions::Vertical),
            _ => Err(OutOfRangeError),
        }
    }
}

const fn validate_width(width: i32) -> Option<usize> {
    if width > 1 {
        #[allow(clippy::cast_sign_loss)]
        Some(width as usize)
    } else {
        None
    }
}

fn blur(data: &mut [u32], width: i32, directions: ffi::Directions) {
    if let (Some(width), Ok(directions)) = (validate_width(width), directions.try_into()) {
        filter::blur(data, width, directions);
    }
}

fn sharpen(data: &mut [u32], width: i32, directions: ffi::Directions) {
    if let (Some(width), Ok(directions)) = (validate_width(width), directions.try_into()) {
        filter::sharpen(data, width, directions);
    }
}

fn edge_detect(data: &mut [u32], width: i32, directions: ffi::Directions) {
    if let (Some(width), Ok(directions)) = (validate_width(width), directions.try_into()) {
        filter::edge_detect(data, width, directions);
    }
}

fn emboss(data: &mut [u32], width: i32, directions: ffi::Directions) {
    if let (Some(width), Ok(directions)) = (validate_width(width), directions.try_into()) {
        filter::emboss(data, width, directions);
    }
}

fn brightness_add(data: &mut [u32], add: i32, channel: ffi::ColorChannel) {
    if let Ok(channel) = channel.try_into() {
        filter::brightness_add(data, add, channel);
    }
}

fn contrast(data: &mut [u32], mult: f64, channel: ffi::ColorChannel) {
    if let Ok(channel) = channel.try_into() {
        filter::contrast(data, mult, channel);
    }
}

fn gamma(data: &mut [u32], exp: f64, channel: ffi::ColorChannel) {
    if let Ok(channel) = channel.try_into() {
        filter::gamma(data, exp, channel);
    }
}

fn brightness_mult(data: &mut [u32], mult: f64, channel: ffi::ColorChannel) {
    if let Ok(channel) = channel.try_into() {
        filter::brightness_mult(data, mult, channel);
    }
}

fn lesser_blur(data: &mut [u32], width: i32, directions: ffi::Directions) {
    if let (Some(width), Ok(directions)) = (validate_width(width), directions.try_into()) {
        filter::lesser_blur(data, width, directions);
    }
}

fn minor_blur(data: &mut [u32], width: i32, directions: ffi::Directions) {
    if let (Some(width), Ok(directions)) = (validate_width(width), directions.try_into()) {
        filter::minor_blur(data, width, directions);
    }
}

#[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
fn color_to_alpha(data: &mut [u32], color: &QColor) {
    let alpha = color.alpha();
    filter::color_to_alpha(
        data,
        Pixel {
            red: (color.red() * alpha / 255) as u8,
            green: (color.green() * alpha / 255) as u8,
            blue: (color.blue() * alpha / 255) as u8,
            alpha: alpha as u8,
        },
    );
}
