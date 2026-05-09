#pragma once
#include "../../casting.h"
#include "miniwindow.h"
#include <QtCore/QRectF>
#include <QtCore/QSize>

namespace geometry {
using Position = MiniWindow::Position;

template<typename T>
concept Point = requires(T t) { t.rx(); };

template<typename T>
concept Size = requires(T t) { t.width(); };

template<typename T>
concept Rect = requires(T t) { t.topLeft(); };

template<typename N>
constexpr N
scale(N value, N scalar) noexcept
{
  return value * scalar;
}

constexpr int
scale(int value, qreal scalar) noexcept
{
  return clamped_cast<int>(value * scalar);
}

template<Point P, Size S>
constexpr P
scale(const P& point, const S& scalar) noexcept
{
  return { scale(point.x(), scalar.width()),
           scale(point.y(), scalar.height()) };
}

template<Rect R, Size S>
constexpr R
scale(const R& rect, const S& scalar) noexcept
{
  return { scale(rect.topLeft(), scalar), scale(rect.bottomRight(), scalar) };
}

template<typename N>
constexpr N
normalize(N n, N limit) noexcept
{
  return n >= 0 ? n : limit - n;
}

template<Point P, Size S>
constexpr P
normalize(const P& point, const S& size) noexcept
{
  return { normalize(point.x(), size.width()),
           normalize(point.y(), size.height()) };
}

template<Rect R, Size S>
constexpr R
normalize(const R& rect, const S& size) noexcept
{
  return { rect.topLeft(), normalize(rect.bottomRight(), size) };
}

qreal
arc(const QPointF& center, const QPointF& edge);

QRect
calculate(Position pos, const QSize& parent, const QSize& child) noexcept;

QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size);

QRect
calculate(QWidget* widget, MiniWindow::Position position);
} // namespace geometry
