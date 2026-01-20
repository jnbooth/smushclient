#include "imagefilters.h"

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
} // namespace

// Public methods

void
BrightnessAddFilter::apply(QPixmap& pixmap) const
{
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  for (uchar *byte = qImage.bits() + rgbaOffset(channel),
             *end = byte + qImage.sizeInBytes();
       byte < end;
       byte += rgbaIncrement(channel)) {
    *byte = static_cast<uchar>(
      std::clamp(static_cast<int>(*byte) + add, 0, UCHAR_MAX));
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
  for (uchar *byte = qImage.bits() + rgbaOffset(channel),
             *end = byte + qImage.sizeInBytes();
       byte < end;
       byte += rgbaIncrement(channel)) {
    *byte = static_cast<uchar>(
      std::min((*byte * multiply), static_cast<qreal>(UCHAR_MAX)));
  }
  pixmap.convertFromImage(qImage);
}

void
GrayscaleFilter::apply(QPixmap& pixmap) const
{
  pixmap.convertFromImage(
    pixmap.toImage().convertToFormat(QImage::Format_Grayscale8));
}
