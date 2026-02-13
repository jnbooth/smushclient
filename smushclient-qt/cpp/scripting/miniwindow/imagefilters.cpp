#include "imagefilters.h"
#include "../../casting.h"
#include "smushclient_qt/src/ffi/filter.cxx.h"

// Private utils

namespace {
ImageFilter::Pixels
asPixels(QImage& image)
{
  return rust::Slice(reinterpret_cast<uint32_t*>(image.bits()),
                     static_cast<size_t>(image.sizeInBytes() >> 2));
}
} // namespace

// Public methods

void
ImageFilter::PixelFilter::apply(QPixmap& pixmap) const
{
  if (isNoop()) {
    return;
  }
  QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  apply(asPixels(image));
  pixmap.convertFromImage(image);
}

void
ImageFilter::ConvolveFilter::apply(QPixmap& pixmap) const
{
  QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  apply(asPixels(image), image.width(), directions);
  pixmap.convertFromImage(image);
}

// Protected methods

void
ImageFilter::Noise::apply(Pixels pixels) const
{
  ffi::filter::noise(pixels, threshold);
}

void
ImageFilter::MonoNoise::apply(Pixels pixels) const
{
  ffi::filter::mono_noise(pixels, threshold);
}

void
ImageFilter::Blur::apply(Pixels pixels, int width, Directions directions) const
{
  ffi::filter::blur(pixels, width, directions);
}

void
ImageFilter::Sharpen::apply(Pixels pixels,
                            int width,
                            Directions directions) const
{
  ffi::filter::sharpen(pixels, width, directions);
}

void
ImageFilter::EdgeDetect::apply(Pixels pixels,
                               int width,
                               Directions directions) const
{
  ffi::filter::edge_detect(pixels, width, directions);
}

void
ImageFilter::Emboss::apply(Pixels pixels,
                           int width,
                           Directions directions) const
{
  ffi::filter::emboss(pixels, width, directions);
}

void
ImageFilter::BrightnessAdd::apply(Pixels pixels) const
{
  ffi::filter::brightness_add(pixels, add, channel);
}

void
ImageFilter::Contrast::apply(Pixels pixels) const
{
  ffi::filter::contrast(pixels, multiply, channel);
}

void
ImageFilter::Gamma::apply(Pixels pixels) const
{
  ffi::filter::gamma(pixels, exp, channel);
}

void
ImageFilter::GrayscaleLinear::apply(Pixels pixels) const
{
  ffi::filter::grayscale_linear(pixels);
}

void
ImageFilter::LesserBlur::apply(Pixels pixels,
                               int width,
                               Directions directions) const
{
  ffi::filter::lesser_blur(pixels, width, directions);
}

void
ImageFilter::MinorBlur::apply(Pixels pixels,
                              int width,
                              Directions directions) const
{
  ffi::filter::minor_blur(pixels, width, directions);
}

void
ImageFilter::GrayscalePerceptual::apply(Pixels pixels) const
{
  ffi::filter::grayscale_perceptual(pixels);
}

void
ImageFilter::BrightnessMult::apply(Pixels pixels) const
{
  ffi::filter::brightness_mult(pixels, multiply, channel);
}

void
ImageFilter::Average::apply(Pixels pixels) const
{
  ffi::filter::average(pixels);
}
