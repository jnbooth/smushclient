#include "imagefilters.h"
#include "../../casting.h"
#include "../../image.h"
#include "smushclient_qt/src/ffi/filter.cxx.h"

// Private utils

// Public methods

QImage
ImageFilter::PixelFilter::apply(const QPixmap& pixmap) const noexcept
{
  if (isNoop() || pixmap.isNull()) {
    return QImage();
  }
  QImage image = pixmap.toImage();
  image.convertTo(QImage::Format::Format_ARGB32);
  apply(image::asPixelsMut(image));
  return image;
}

QImage
ImageFilter::ConvolveFilter::apply(const QPixmap& pixmap) const noexcept
{
  QImage image = pixmap.toImage();
  image.convertTo(QImage::Format::Format_ARGB32);
  apply(image::asPixelsMut(image), image.width(), directions);
  return image;
}

// Protected methods

void
ImageFilter::Noise::apply(Pixels pixels) const noexcept
{
  ffi::filter::noise(pixels, threshold);
}

void
ImageFilter::MonoNoise::apply(Pixels pixels) const noexcept
{
  ffi::filter::mono_noise(pixels, threshold);
}

void
ImageFilter::Blur::apply(Pixels pixels,
                         int width,
                         Directions directions) const noexcept
{
  ffi::filter::blur(pixels, width, directions);
}

void
ImageFilter::Sharpen::apply(Pixels pixels,
                            int width,
                            Directions directions) const noexcept
{
  ffi::filter::sharpen(pixels, width, directions);
}

void
ImageFilter::EdgeDetect::apply(Pixels pixels,
                               int width,
                               Directions directions) const noexcept
{
  ffi::filter::edge_detect(pixels, width, directions);
}

void
ImageFilter::Emboss::apply(Pixels pixels,
                           int width,
                           Directions directions) const noexcept
{
  ffi::filter::emboss(pixels, width, directions);
}

void
ImageFilter::BrightnessAdd::apply(Pixels pixels) const noexcept
{
  ffi::filter::brightness_add(pixels, add, channel);
}

void
ImageFilter::Contrast::apply(Pixels pixels) const noexcept
{
  ffi::filter::contrast(pixels, multiply, channel);
}

void
ImageFilter::Gamma::apply(Pixels pixels) const noexcept
{
  ffi::filter::gamma(pixels, exp, channel);
}

void
ImageFilter::GrayscaleLinear::apply(Pixels pixels) const noexcept
{
  ffi::filter::grayscale_linear(pixels);
}

void
ImageFilter::LesserBlur::apply(Pixels pixels,
                               int width,
                               Directions directions) const noexcept
{
  ffi::filter::lesser_blur(pixels, width, directions);
}

void
ImageFilter::MinorBlur::apply(Pixels pixels,
                              int width,
                              Directions directions) const noexcept
{
  ffi::filter::minor_blur(pixels, width, directions);
}

void
ImageFilter::GrayscalePerceptual::apply(Pixels pixels) const noexcept
{
  ffi::filter::grayscale_perceptual(pixels);
}

void
ImageFilter::BrightnessMult::apply(Pixels pixels) const noexcept
{
  ffi::filter::brightness_mult(pixels, multiply, channel);
}

void
ImageFilter::Average::apply(Pixels pixels) const noexcept
{
  ffi::filter::average(pixels);
}
