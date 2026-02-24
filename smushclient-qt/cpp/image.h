#pragma once
#include "rust/cxx.h"
#include <QtGui/QImage>
#include <QtGui/QPixmap>

enum class BlendMode : int64_t;

namespace image {
QImage::Format
_getBitmapFormat() noexcept;

const QImage::Format bitmapFormat = _getBitmapFormat();

inline rust::Slice<const uint8_t>
asBytes(const QImage& image) noexcept
{
  return rust::Slice(static_cast<const uint8_t*>(image.bits()),
                     static_cast<size_t>(image.sizeInBytes()));
}

inline rust::Slice<uint8_t>
asBytesMut(QImage& image) noexcept
{
  return rust::Slice(static_cast<uint8_t*>(image.bits()),
                     static_cast<size_t>(image.sizeInBytes()));
}

inline rust::Slice<const uint32_t>
asPixels(const QImage& image) noexcept
{
  return rust::Slice(reinterpret_cast<const uint32_t*>(image.bits()),
                     static_cast<size_t>(image.sizeInBytes() >> 2));
}

inline rust::Slice<uint32_t>
asPixelsMut(QImage& image) noexcept
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

void
colorToAlpha(QImage& image, const QColor& color);

QImage
crop(const QPixmap& pixmap, const QRect& rect);

QBitmap
invertBitmap(const QPixmap& base);

bool
mask(QImage& image, QImage& mask, qreal opacity);

QRgb
topLeftPixel(const QImage& image);

QRgb
topLeftPixel(const QPixmap& pixmap);
} // namespace image
