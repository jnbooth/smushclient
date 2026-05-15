#pragma once
#include "../../casting.h"
#include "miniwindow.h"
#include <QtCore/QRectF>
#include <QtCore/QSize>

namespace geometry {
using Position = MiniWindow::Position;

template<typename P>
concept Point = std::is_same_v<P, QPoint> || std::is_same_v<P, QPointF>;

template<typename R>
concept Rect = std::is_same_v<R, QRect> || std::is_same_v<R, QRectF>;

template<typename N>
concept Scalar = std::is_arithmetic_v<N>;

template<typename S>
concept Size = std::is_same_v<S, QSize> || std::is_same_v<S, QSizeF>;

template<Scalar N>
constexpr N
scale(N value, N scaleBy) noexcept
{
  return value * scaleBy;
}

constexpr int
scale(int value, qreal scaleBy) noexcept
{
  return clamped_cast<int>(value * scaleBy);
}

template<Point P, Size S>
constexpr P
scale(const P& point, const S& scaleBy) noexcept
{
  return { scale(point.x(), scaleBy.width()),
           scale(point.y(), scaleBy.height()) };
}

template<Rect R, Size S>
constexpr R
scale(const R& rect, const S& scaleBy) noexcept
{
  return { scale(rect.topLeft(), scaleBy), scale(rect.bottomRight(), scaleBy) };
}

template<Scalar N>
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
arc(const QPointF& center, const QPointF& edge) noexcept;

QRect
calculate(Position pos, const QSize& parent, const QSize& child) noexcept;

QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size);

QRect
calculate(QWidget* widget, MiniWindow::Position position);
} // namespace geometry
