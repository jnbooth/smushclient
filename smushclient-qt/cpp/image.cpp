#include "image.h"
#include "scripting/scriptenums.h"
#include "smushclient_qt/src/ffi/filter.cxx.h"
#include <QtGui/QPainter>

// Private utils

namespace {
QImage
dissolve(const QPixmap& source, const QRect& rect, qreal opacity)
{

  QImage image =
    source.rect() == rect ? source.toImage() : source.copy(rect).toImage();
  image.convertTo(QImage::Format::Format_ARGB32);
  ffi::filter::dissolve(image::asPixels(image), opacity);
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

  switch (mode) {
    case BlendMode::Normal:
      break;
    case BlendMode::Average:
    case BlendMode::Interpolate:
      opacity /= 2;
      break;
    case BlendMode::Dissolve:
      painter.drawImage(origin, dissolve(source, sourceRect.toRect(), opacity));
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
  painter.drawPixmap(origin, source, sourceRect);
  return true;
}
} // namespace image
