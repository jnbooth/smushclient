use crate::dimensions::{StepSlice, StepSliceMut, SubpixelImage};

#[derive(Copy, Clone, Debug, Default, PartialEq, Eq)]
pub enum Directions {
    #[default]
    Both = 0,
    Horizontal,
    Vertical,
}

pub(crate) fn convolve(data: &mut [u32], width: usize, directions: Directions, matrix: &[f64]) {
    let mut input_buf = Vec::new();
    let mut output = SubpixelImage::from_argb(data, width);
    let mut input = output.encode_f64(&mut input_buf);
    if directions != Directions::Horizontal {
        for col in 0..output.width {
            convolve_line(&input.column(col), &mut output.column_mut(col), matrix);
        }
    }
    if directions == Directions::Both {
        input = output.encode_f64(&mut input_buf);
    }
    if directions != Directions::Vertical {
        for row in 0..output.height {
            for ch in 0..4 {
                convolve_line(&input.row(row, ch), &mut output.row_mut(row, ch), matrix);
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

#[allow(clippy::cast_possible_truncation, clippy::cast_sign_loss)]
impl FloatConvert for u8 {
    #[inline(always)]
    fn to_f64(self) -> f64 {
        f64::from(self)
    }
    #[inline(always)]
    fn from_f64(f: f64) -> Self {
        f as u8
    }
}

#[inline]
fn convolve_line<I, O>(input: &StepSlice<I>, output: &mut StepSliceMut<O>, matrix: &[f64])
where
    I: FloatConvert,
    O: FloatConvert,
{
    debug_assert!(matrix.len() % 2 == 1, "matrix length is even");
    let midpoint = 1 + matrix.len() / 2;
    for (position, subpixel) in output.into_iter().enumerate() {
        let total = matrix
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
