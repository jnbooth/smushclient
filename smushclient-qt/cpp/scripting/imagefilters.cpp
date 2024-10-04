#include "imagefilters.h"

// Private utils

constexpr int rgbaOffset(ImageFilter::Channel channel) noexcept
{
  if (channel == ImageFilter::Channel::All)
    return 0;
  if constexpr (std::endian::native == std::endian::big)
    return 2 - ((int)channel); // A R G B
  else
    return ((int)channel); // B G R A
}

constexpr int rgbaIncrement(ImageFilter::Channel channel) noexcept
{
  return channel == ImageFilter::Channel::All ? 1 : 4;
}

// Public methods

void BrightnessAddFilter::apply(QPixmap &pixmap) const
{
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  for (
      uchar *byte = qImage.bits() + rgbaOffset(channel), *end = byte + qImage.sizeInBytes();
      byte < end;
      byte += rgbaIncrement(channel))
    *byte = (uchar)std::clamp((int)*byte + add, 0, UCHAR_MAX);
  pixmap.convertFromImage(qImage);
}

void BrightnessMultFilter::apply(QPixmap &pixmap) const
{
  if (multiply < 0)
    return;
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  for (
      uchar *byte = qImage.bits() + rgbaOffset(channel), *end = byte + qImage.sizeInBytes();
      byte < end;
      byte += rgbaIncrement(channel))
    *byte = (uchar)std::min((*byte * multiply), (float)UCHAR_MAX);
  pixmap.convertFromImage(qImage);
}

void GrayscaleFilter::apply(QPixmap &pixmap) const
{
  pixmap.convertFromImage(pixmap.toImage().convertToFormat(QImage::Format_Grayscale8));
}
