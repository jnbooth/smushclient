#![allow(clippy::needless_for_each)]

use crate::ColorChannel;
use crate::casting::{as_bytes_mut, as_pixels_mut};
use crate::pixel::Pixel;

#[inline(always)]
const fn is_rgb_channel(index: usize) -> bool {
    index & 3 != ColorChannel::Alpha as usize
}

pub(crate) fn adjust_subpixels<F>(data: &mut [u32], channel: Option<ColorChannel>, f: F)
where
    F: Fn(u8) -> u8,
{
    adjust_subpixels_with_state(data, channel, f);
}

pub(crate) fn adjust_subpixels_with_state<F>(
    data: &mut [u32],
    channel: Option<ColorChannel>,
    mut f: F,
) where
    F: FnMut(u8) -> u8,
{
    if data.is_empty() {
        return;
    }
    match channel {
        Some(channel) => as_bytes_mut(data)[channel as usize..]
            .iter_mut()
            .step_by(4)
            .for_each(|c| *c = f(*c)),
        None => as_bytes_mut(data)
            .iter_mut()
            .enumerate()
            .for_each(|(i, c)| {
                if is_rgb_channel(i) {
                    *c = f(*c);
                }
            }),
    }
}

#[allow(clippy::cast_possible_truncation)]
pub(crate) fn adjust_subpixels_cached<F>(data: &mut [u32], channel: Option<ColorChannel>, f: F)
where
    F: Fn(u8) -> u8,
{
    let mut cache = [0; 256];
    for (i, value) in cache.iter_mut().enumerate() {
        *value = f(i as u8);
    }
    // SAFETY: `cache` has length 256, so any u8 is a valid index.
    adjust_subpixels(data, channel, |sp| unsafe {
        *cache.get_unchecked(sp as usize)
    });
}

pub(crate) fn adjust_pixels<F>(data: &mut [u32], f: F)
where
    F: Fn(Pixel) -> Pixel,
{
    adjust_pixels_with_state(data, f);
}

pub(crate) fn adjust_pixels_with_state<F>(data: &mut [u32], mut f: F)
where
    F: FnMut(Pixel) -> Pixel,
{
    as_pixels_mut(data).iter_mut().for_each(|p| *p = f(*p));
}
