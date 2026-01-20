#include "imagefilters.h"

using std::span;

// Private utils

namespace {
constexpr int
rgbaOffset(ImageFilter::Channel channel) noexcept
{
  if (channel == ImageFilter::Channel::All) {
    return 0;
  }
  if constexpr (std::endian::native == std::endian::big) {
    return 2 - channel; // A R G B
  } else {
    return channel; // B G R A
  }
}

constexpr int
rgbaIncrement(ImageFilter::Channel channel) noexcept
{
  return channel == ImageFilter::Channel::All ? 1 : 4;
}

span<uchar>
getImageBits(QImage& image, ImageFilter::Channel channel)
{
  return span(image.bits(), image.sizeInBytes()).subspan(rgbaOffset(channel));
}
} // namespace

// Public methods

void
BrightnessAddFilter::apply(QPixmap& pixmap) const
{
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  span<uchar> imageBits =
    span(qImage.bits(), qImage.sizeInBytes()).subspan(rgbaOffset(channel));
  const int increment = rgbaIncrement(channel);
  for (auto iter = imageBits.begin(), end = imageBits.end(); iter < end;
       iter += increment) {
    *iter = static_cast<uchar>(
      std::clamp(static_cast<int>(*iter) + add, 0, UCHAR_MAX));
  }
  pixmap.convertFromImage(qImage);
}

void
BrightnessMultFilter::apply(QPixmap& pixmap) const
{
  if (multiply < 0) {
    return;
  }
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  const int increment = rgbaIncrement(channel);
  span<uchar> imageBits = getImageBits(qImage, channel);
  for (auto iter = imageBits.begin(), end = imageBits.end(); iter < end;
       iter += increment) {
    *iter = static_cast<uchar>(
      std::min((*iter * multiply), static_cast<qreal>(UCHAR_MAX)));
  }
  pixmap.convertFromImage(qImage);
}

void
GrayscaleFilter::apply(QPixmap& pixmap) const
{
  pixmap.convertFromImage(
    pixmap.toImage().convertToFormat(QImage::Format_Grayscale8));
}
