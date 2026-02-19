#pragma once
#include "rust/cxx.h"
#include <QtGui/QImage>
#include <QtGui/QPixmap>

enum class BlendMode : int64_t;

namespace image {
inline rust::Slice<const uint32_t>
asPixels(const QImage& image)
{
  return rust::Slice(reinterpret_cast<const uint32_t*>(image.bits()),
                     static_cast<size_t>(image.sizeInBytes() >> 2));
}

inline rust::Slice<uint32_t>
asPixels(QImage& image)
{
  return rust::Slice(reinterpret_cast<uint32_t*>(image.bits()),
                     static_cast<size_t>(image.sizeInBytes() >> 2));
}

bool
blend(QPixmap& target,
      const QPixmap& source,
      const QPointF& origin,
      BlendMode mode,
      qreal opacity,
      const QRectF& sourceRect);
} // namespace image
