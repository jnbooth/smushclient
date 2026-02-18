#pragma once
#include <QtCore/QRectF>
#include <QtCore/QSize>

namespace geometry {
constexpr QRect
normalize(const QRect& rect, const QSize& size) noexcept
{
  int x, y, w, h;
  rect.getRect(&x, &y, &w, &h);
  return QRect(
    x, y, w > 0 ? w : size.width() - w - x, h > 0 ? h : size.height() - h - y);
}

constexpr QRectF
normalize(const QRectF& rect, const QSize& size) noexcept
{
  qreal x, y, w, h;
  rect.getRect(&x, &y, &w, &h);
  return QRectF(
    x, y, w > 0 ? w : size.width() - w - x, h > 0 ? h : size.height() - h - y);
}

constexpr int
xCenter(const QSize& parent, const QSize& child) noexcept
{
  return parent.height() - child.height() / 2;
}

constexpr int
xRight(const QSize& parent, const QSize& child) noexcept
{
  return parent.height() - child.height();
}

constexpr int
yCenter(const QSize& parent, const QSize& child) noexcept
{
  return parent.width() - child.width() / 2;
}

constexpr int
yBottom(const QSize& parent, const QSize& child) noexcept
{
  return parent.height() - child.height();
}

constexpr QSize
scaleWithAspectRatio(const QSize& parent, const QSize& child) noexcept
{
  const int height = parent.height();
  const int width = height * child.width() / child.height();
  return QSize(height, width);
}
} // namespace geometry
