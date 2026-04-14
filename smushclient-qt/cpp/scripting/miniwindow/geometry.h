#pragma once
#include "../../casting.h"
#include "miniwindow.h"
#include <QtCore/QRectF>
#include <QtCore/QSize>

namespace geometry {
using Position = MiniWindow::Position;

qreal
arc(const QPointF& center, const QPointF& edge);

constexpr int
scale(int value, qreal scalar) noexcept
{
  return clamped_cast<int>(value * scalar);
}

constexpr QPoint
scale(const QPoint& point, const QSizeF& scalar) noexcept
{
  return QPoint(scale(point.x(), scalar.width()),
                scale(point.y(), scalar.height()));
}

constexpr QRect
scale(const QRect& rect, const QSizeF& scalar) noexcept
{
  return QRect(scale(rect.topLeft(), scalar),
               scale(rect.bottomRight(), scalar));
}

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

QRect
calculate(Position pos, const QSize& parent, const QSize& child) noexcept;

QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size);

QRect
calculate(QWidget* widget, MiniWindow::Position position);
} // namespace geometry
