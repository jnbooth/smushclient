use crate::Pixel;

#[inline(always)]
pub(crate) fn as_bytes_mut(data: &mut [u32]) -> &mut [u8] {
    bytemuck::must_cast_slice_mut(data)
}

#[inline(always)]
pub(crate) fn as_pixels(data: &[u32]) -> &[Pixel] {
    bytemuck::must_cast_slice(data)
}

#[inline(always)]
pub(crate) fn as_pixels_mut(data: &mut [u32]) -> &mut [Pixel] {
    bytemuck::must_cast_slice_mut(data)
}
