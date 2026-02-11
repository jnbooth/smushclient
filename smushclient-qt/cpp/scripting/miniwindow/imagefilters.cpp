#include "imagefilters.h"
#include "imagebits.h"

// Public methods

void
BrightnessAddFilter::apply(QPixmap& pixmap) const
{
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  for (uchar& imageBit : ImageBitsRange(qImage, channel)) {
    imageBit = static_cast<uchar>(
      std::clamp(static_cast<int>(imageBit) + add, 0, UCHAR_MAX));
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
  for (uchar& imageBit : ImageBitsRange(qImage, channel)) {
    imageBit = static_cast<uchar>(
      std::min((imageBit * multiply), static_cast<qreal>(UCHAR_MAX)));
  }
  pixmap.convertFromImage(qImage);
}

void
GrayscaleFilter::apply(QPixmap& pixmap) const
{
  pixmap.convertFromImage(
    pixmap.toImage().convertToFormat(QImage::Format_Grayscale8));
}
