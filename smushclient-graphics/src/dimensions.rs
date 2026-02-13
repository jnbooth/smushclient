use std::ops::Range;
use std::{iter, slice};

use crate::casting::as_bytes_mut;

pub(crate) struct SubpixelImage<'a, T> {
    pub data: &'a mut [T],
    pub width: usize,
    pub height: usize,
}

impl<'a> SubpixelImage<'a, u8> {
    pub fn from_argb(data: &'a mut [u32], width: usize) -> Self {
        debug_assert_eq!(data.len() % width, 0, "invalid width");
        let height = data.len() / width;
        Self {
            data: as_bytes_mut(data),
            width: width * 4,
            height,
        }
    }
}

impl SubpixelImage<'_, u8> {
    pub fn encode_f64<'a>(&self, buf: &'a mut Vec<f64>) -> SubpixelImage<'a, f64> {
        buf.clear();
        buf.extend(self.data.iter().copied().map(f64::from));
        SubpixelImage {
            data: buf,
            width: self.width,
            height: self.height,
        }
    }
}

impl<T> SubpixelImage<'_, T> {
    #[inline(always)]
    fn row_range(&self, i: usize, channel: usize) -> Range<usize> {
        debug_assert!(i < self.height);
        let start = i * self.width;
        start + channel..start + self.width + channel - 3
    }

    #[inline(always)]
    fn col_range(&self, i: usize) -> Range<usize> {
        debug_assert!(i < self.width);
        i..i + self.data.len() + 1 - self.width
    }

    /// `i` is assumed to be less than `self.height`, and `channel` is assumed to be less than 4.
    /// If that is not the case, logic errors or panics may occur.
    #[inline]
    pub fn row(&self, i: usize, channel: usize) -> StepSlice<'_, T> {
        let range = self.row_range(i, channel);
        StepSlice {
            data: &self.data[range],
            step: 4,
        }
    }

    /// `i` is assumed to be less than `self.height`, and `channel` is assumed to be less than 4.
    /// If that is not the case, logic errors or panics may occur.
    #[inline]
    pub fn row_mut(&mut self, i: usize, channel: usize) -> StepSliceMut<'_, T> {
        let range = self.row_range(i, channel);
        StepSliceMut {
            data: &mut self.data[range],
            skip: 4,
        }
    }

    /// `i` is assumed to be less than `self.width`. If that is not the case, logic errors or
    /// panics may occur.
    #[inline]
    pub fn column(&self, i: usize) -> StepSlice<'_, T> {
        let range = self.col_range(i);
        StepSlice {
            data: &self.data[range],
            step: self.width,
        }
    }

    /// `i` is assumed to be less than `self.width`. If that is not the case, logic errors or
    /// panics may occur.
    #[inline]
    pub fn column_mut(&mut self, i: usize) -> StepSliceMut<'_, T> {
        let range = self.col_range(i);
        StepSliceMut {
            data: &mut self.data[range],
            skip: self.width,
        }
    }
}

pub(crate) struct StepSlice<'a, T> {
    data: &'a [T],
    step: usize,
}

impl<T: Copy> StepSlice<'_, T> {
    #[inline(always)]
    pub fn get_clamped(&self, i: usize) -> T {
        match self.data.get(i * self.step) {
            Some(&val) => val,
            None => *self.data.last().unwrap(),
        }
    }
}

pub(crate) struct StepSliceMut<'a, T> {
    data: &'a mut [T],
    skip: usize,
}

impl<'a, T> IntoIterator for &'a mut StepSliceMut<'_, T> {
    type Item = &'a mut T;

    type IntoIter = iter::StepBy<slice::IterMut<'a, T>>;

    #[inline]
    fn into_iter(self) -> Self::IntoIter {
        self.data.iter_mut().step_by(self.skip)
    }
}
