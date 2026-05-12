#pragma once
#include "../../casting.h"
#include "miniwindow.h"
#include <QtCore/QRectF>
#include <QtCore/QSize>

namespace geometry {
using Position = MiniWindow::Position;

template<typename T, typename Arg, typename... Args>
constexpr bool is_one_of_v = std::is_same_v<T, Arg> || is_one_of_v<T, Args...>;

template<typename T, typename Arg>
constexpr bool is_one_of_v<T, Arg> = std::is_same_v<T, Arg>;

template<typename P>
concept Point = is_one_of_v<P, QPoint, QPointF>;

template<typename R>
concept Rect = is_one_of_v<R, QRect, QRectF>;

template<typename N>
concept Scalar = std::is_arithmetic_v<N>;

template<typename S>
concept Size = is_one_of_v<S, QSize, QSizeF>;

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
arc(const QPointF& center, const QPointF& edge);

QRect
calculate(Position pos, const QSize& parent, const QSize& child) noexcept;

QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size);

QRect
calculate(QWidget* widget, MiniWindow::Position position);
} // namespace geometry
