use std::ops::Range;

use crate::channel::ColorChannel;
use crate::dimensions::{StepSlice, StepSliceMut, SubpixelImage};

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq)]
pub enum Directions {
    #[default]
    Both = 0,
    Horizontal,
    Vertical,
}

#[inline(always)]
const fn is_rgb_column(index: usize) -> bool {
    index & 3 != ColorChannel::Alpha as usize
}

pub(crate) fn convolve(data: &mut [u32], width: usize, directions: Directions, kernel: &[f64]) {
    #[cfg(target_endian = "little")]
    const RGB_CHANNELS: Range<usize> = 0..3;
    #[cfg(target_endian = "big")]
    const RGB_CHANNELS: Range<usize> = 1..4;

    debug_assert!(kernel.len() % 2 == 1, "kernel length is even");

    let mut input_buf = Vec::new();
    let mut output = SubpixelImage::from_argb(data, width);
    let mut input = output.encode_f64(&mut input_buf);
    if directions != Directions::Horizontal {
        for col in 0..output.width() {
            if is_rgb_column(col) {
                convolve_line(&input.column(col), &mut output.column_mut(col), kernel);
            }
        }
    }
    if directions == Directions::Both {
        input = output.encode_f64(&mut input_buf);
    }
    if directions != Directions::Vertical {
        for row in 0..output.height() {
            for ch in RGB_CHANNELS {
                convolve_line(&input.row(row, ch), &mut output.row_mut(row, ch), kernel);
            }
        }
    }
}

trait FloatConvert: Copy {
    fn to_f64(self) -> f64;
    fn from_f64(f: f64) -> Self;
}

impl FloatConvert for f64 {
    #[inline(always)]
    fn to_f64(self) -> f64 {
        self
    }
    #[inline(always)]
    fn from_f64(f: f64) -> Self {
        f
    }
}

impl FloatConvert for u8 {
    #[inline(always)]
    fn to_f64(self) -> f64 {
        f64::from(self)
    }
    #[inline(always)]
    #[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
    fn from_f64(f: f64) -> Self {
        f as u8
    }
}

#[inline]
fn convolve_line<I, O>(input: &StepSlice<I>, output: &mut StepSliceMut<O>, kernel: &[f64])
where
    I: FloatConvert,
    O: FloatConvert,
{
    let midpoint = 1 + kernel.len() / 2;
    for (position, subpixel) in output.into_iter().enumerate() {
        let total = kernel
            .iter()
            .enumerate()
            .map(|(i, weight)| {
                let index = (position + i).saturating_sub(midpoint);
                weight * input.get_clamped(index).to_f64()
            })
            .sum();
        *subpixel = FloatConvert::from_f64(total);
    }
}
