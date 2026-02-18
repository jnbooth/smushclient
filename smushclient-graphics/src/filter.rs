#![allow(
    clippy::cast_possible_truncation,
    clippy::cast_lossless,
    clippy::cast_sign_loss
)]
use crate::casting::as_pixels;
use crate::channel::ColorChannel;
use crate::convolve::{Directions, convolve};
use crate::iter::{
    adjust_pixels, adjust_pixels_with_state, adjust_subpixels, adjust_subpixels_cached,
    adjust_subpixels_with_state, channel_subpixels_mut,
};
use crate::pixel::Pixel;
use crate::random::{DissolveRng, NoiseRng};

pub fn noise(data: &mut [u32], threshold: f64) {
    let mut rng = NoiseRng::new(threshold);
    adjust_subpixels_with_state(data, None, |sp| (sp as f64 + rng.next()) as u8);
}

pub fn mono_noise(data: &mut [u32], threshold: f64) {
    let mut rng = NoiseRng::new(threshold);
    adjust_pixels_with_state(data, |p| {
        let noise = rng.next();
        Pixel {
            blue: (p.blue as f64 + noise) as u8,
            green: (p.green as f64 + noise) as u8,
            red: (p.red as f64 + noise) as u8,
            alpha: p.alpha,
        }
    });
}

pub fn blur(data: &mut [u32], width: usize, directions: Directions) {
    const KERNEL: [f64; 5] = [0.2, 0.2, 0.2, 0.2, 0.2];
    convolve(data, width, directions, &KERNEL);
}

pub fn sharpen(data: &mut [u32], width: usize, directions: Directions) {
    const KERNEL: [f64; 5] = [-1.0 / 3.0, -1.0 / 3.0, 7.0 / 3.0, -1.0 / 3.0, -1.0 / 3.0];
    convolve(data, width, directions, &KERNEL);
}

pub fn edge_detect(data: &mut [u32], width: usize, directions: Directions) {
    const KERNEL: [f64; 5] = [0.0, 2.5, -6.0, 2.5, 0.0];
    convolve(data, width, directions, &KERNEL);
}

pub fn emboss(data: &mut [u32], width: usize, directions: Directions) {
    const KERNEL: [f64; 5] = [1.0, 2.0, 1.0, -1.0, -2.0];
    convolve(data, width, directions, &KERNEL);
}

pub fn brightness_add(data: &mut [u32], add: i32, channel: Option<ColorChannel>) {
    if add >= 0 {
        let add = add.try_into().unwrap_or(u8::MAX);
        adjust_subpixels(data, channel, |sp| sp.saturating_add(add));
    } else {
        let sub = (-add).try_into().unwrap_or(u8::MAX);
        adjust_subpixels(data, channel, |sp| sp.saturating_sub(sub));
    }
}

pub fn contrast(data: &mut [u32], mult: f64, channel: Option<ColorChannel>) {
    adjust_subpixels_cached(data, channel, |sp| {
        ((sp as f64 - 128.0) * mult + 128.0) as u8
    });
}

pub fn gamma(data: &mut [u32], exp: f64, channel: Option<ColorChannel>) {
    adjust_subpixels_cached(data, channel, |sp| {
        ((sp as f64 / 255.0).powf(exp) * 255.0) as u8
    });
}

pub fn grayscale_linear(data: &mut [u32]) {
    adjust_pixels(data, |p| {
        let average = ((p.red as u16 + p.green as u16 + p.blue as u16) / 3) as u8;
        Pixel {
            blue: average,
            green: average,
            red: average,
            alpha: p.alpha,
        }
    });
}

pub fn grayscale_perceptual(data: &mut [u32]) {
    adjust_pixels(data, |p| {
        let average = (p.blue as f64 * 0.11 + p.green as f64 * 0.59 + p.red as f64 * 0.3) as u8;
        Pixel {
            blue: average,
            green: average,
            red: average,
            alpha: p.alpha,
        }
    });
}

pub fn brightness_mult(data: &mut [u32], mult: f64, channel: Option<ColorChannel>) {
    adjust_subpixels(data, channel, |sp| (sp as f64 * mult) as u8);
}

pub fn lesser_blur(data: &mut [u32], width: usize, directions: Directions) {
    const KERNEL: [f64; 5] = [0.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0, 0.0];
    convolve(data, width, directions, &KERNEL);
}

pub fn minor_blur(data: &mut [u32], width: usize, directions: Directions) {
    const KERNEL: [f64; 5] = [0.0, 0.25, 0.5, 0.25, 0.0];
    convolve(data, width, directions, &KERNEL);
}

pub fn average(data: &mut [u32]) {
    let (blue, green, red) = as_pixels(data).iter().fold((0, 0, 0), |(b, g, r), p| {
        (b + p.blue as u64, g + p.green as u64, r + p.red as u64)
    });
    let len = data.len() as u64;
    let average_pixel = Pixel {
        blue: (blue / len) as u8,
        green: (green / len) as u8,
        red: (red / len) as u8,
        alpha: 1,
    };
    adjust_pixels(data, |_| average_pixel);
}

pub fn dissolve(data: &mut [u32], opacity: f64) {
    let mut rng = DissolveRng::new(opacity);
    for subpixel in channel_subpixels_mut(data, ColorChannel::Alpha) {
        if rng.erase() {
            *subpixel = 0;
        }
    }
}
