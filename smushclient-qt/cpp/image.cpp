#include "image.h"
#include "scripting/scriptenums.h"
#include "smushclient_qt/src/ffi/filter.cxx.h"
#include <QtGui/QBitmap>
#include <QtGui/QImage>
#include <QtGui/QPainter>

using std::span;

// Private utils

namespace {
QImage
dissolve(const QPixmap& source, const QRect& rect, qreal opacity)
{
  QImage image = image::crop(source, rect);
  image.convertTo(QImage::Format::Format_ARGB32);
  ffi::filter::dissolve(image::asPixelsMut(image), opacity);
  return image;
}

} // namespace

// Public functions

namespace image {
bool
blend(QPixmap& target,
      const QPixmap& source,
      const QPointF& origin,
      BlendMode mode,
      qreal opacity,
      const QRectF& sourceRect)
{
  QPainter painter(&target);
  const QRectF rect =
    sourceRect.isNull() ? source.rect().toRectF() : sourceRect;

  switch (mode) {
    case BlendMode::Normal:
      break;
    case BlendMode::Average:
    case BlendMode::Interpolate:
      opacity /= 2;
      break;
    case BlendMode::Dissolve:
      painter.drawImage(origin, dissolve(source, rect.toRect(), opacity));
      return true;
    case BlendMode::Darken:
      painter.setCompositionMode(QPainter::CompositionMode_Darken);
      break;
    case BlendMode::Multiply:
      painter.setCompositionMode(QPainter::CompositionMode_Multiply);
      break;
    case BlendMode::ColorBurn:
      painter.setCompositionMode(QPainter::CompositionMode_ColorBurn);
      break;
    case BlendMode::Lighten:
      painter.setCompositionMode(QPainter::CompositionMode_Lighten);
      break;
    case BlendMode::Screen:
      painter.setCompositionMode(QPainter::CompositionMode_Screen);
      break;
    case BlendMode::ColorDodge:
      painter.setCompositionMode(QPainter::CompositionMode_ColorDodge);
      break;
    case BlendMode::LinearDodge:
    case BlendMode::Add:
      painter.setCompositionMode(QPainter::CompositionMode_Plus);
      break;
    case BlendMode::Overlay:
      painter.setCompositionMode(QPainter::CompositionMode_Overlay);
      break;
    case BlendMode::SoftLight:
      painter.setCompositionMode(QPainter::CompositionMode_SoftLight);
      break;
    case BlendMode::HardLight:
      painter.setCompositionMode(QPainter::CompositionMode_HardLight);
      break;
    case BlendMode::Difference:
      painter.setCompositionMode(QPainter::CompositionMode_Difference);
      break;
    case BlendMode::Exclusion:
      painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
      break;
    case BlendMode::Xor:
      painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
      break;
    case BlendMode::And:
      painter.setCompositionMode(QPainter::RasterOp_SourceAndDestination);
      break;
    case BlendMode::Or:
      painter.setCompositionMode(QPainter::RasterOp_SourceOrDestination);
      break;
    default:
      return false;
  }
  painter.setOpacity(opacity);
  painter.drawPixmap(origin, source, rect);
  return true;
}

QPixmap
colorPixel(const QColor& color)
{
  QPixmap pixmap(1, 1);
  pixmap.fill(color);
  return pixmap;
}

void
colorToAlpha(QImage& image, const QColor& color)
{
  image.convertTo(QImage::Format::Format_ARGB32_Premultiplied);
  ffi::filter::color_to_alpha(asPixelsMut(image), color);
}

QImage
crop(const QPixmap& pixmap, const QRect& rect)
{
  return rect == pixmap.rect() ? pixmap.toImage() : pixmap.copy(rect).toImage();
}

QBitmap
invertBitmap(const QPixmap& base)
{
  QImage image = base.toImage();
  image.convertTo(bitmapFormat);
  span<unsigned char> bytes(image.bits(), image.sizeInBytes());
  for (unsigned char& byte : bytes) {
    byte = ~byte;
  }
  return QBitmap::fromData(base.size(), bytes.data(), bitmapFormat);
}

bool
mask(QImage& image, QImage& mask, qreal opacity)
{
  image.convertTo(QImage::Format::Format_ARGB32_Premultiplied);
  mask.convertTo(QImage::Format::Format_Grayscale8);
  return ffi::filter::mask_premultiplied(
    asPixelsMut(image), asBytes(mask), opacity);
}

QRgb
topLeftPixel(const QImage& image)
{
  if (image.size().isEmpty()) {
    return QRgb();
  }
  return image.pixel(0, 0);
}

QRgb
topLeftPixel(const QPixmap& pixmap)
{
  if (pixmap.size().isEmpty()) {
    return QRgb();
  }
  return pixmap.copy(QRect(0, 0, 1, 1)).toImage().pixel(0, 0);
}
} // namespace image
