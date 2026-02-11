#include "imagefilters.h"
#include "../../casting.h"
#include "imagebits.h"

// Public methods

void
BrightnessAddFilter::apply(QPixmap& pixmap) const
{
  QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB32);
  for (uchar& imageBit : ImageBitsRange(qImage, channel)) {
    imageBit = clamped_cast<uchar>(static_cast<int>(imageBit) + add);
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
    imageBit = clamped_cast<uchar>(imageBit * multiply);
  }
  pixmap.convertFromImage(qImage);
}

void
GrayscaleFilter::apply(QPixmap& pixmap) const
{
  pixmap.convertFromImage(
    pixmap.toImage().convertToFormat(QImage::Format_Grayscale8));
}
