#pragma once
#include "miniwindow.h"
#include <QtCore/QRectF>
#include <QtCore/QSize>

namespace geometry {
using Position = MiniWindow::Position;

constexpr int
scale(int value, qreal scalar) noexcept
{
  return static_cast<int>(static_cast<qreal>(value) * scalar);
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

constexpr QRect
calculate(Position pos, const QSize& parent, const QSize& child) noexcept
{
  switch (pos) {
    case Position::OutputStretch:
    case Position::OwnerStretch:
    case Position::Tile:
      return { {}, parent };
    case Position::OutputScale:
    case Position::OwnerScale:
      return { {}, scaleWithAspectRatio(parent, child) };
    case Position::TopLeft:
      return { {}, child };
    case Position::TopCenter:
      return { { xCenter(parent, child), 0 }, child };
    case Position::TopRight:
      return { { xRight(parent, child), 0 }, child };
    case Position::CenterRight:
      return { { xRight(parent, child), yCenter(parent, child) }, child };
    case Position::BottomRight:
      return { { xRight(parent, child), yBottom(parent, child) }, child };
    case Position::BottomCenter:
      return { { xCenter(parent, child), yBottom(parent, child) }, child };
    case Position::BottomLeft:
      return { { 0, yBottom(parent, child) }, child };
    case Position::CenterLeft:
      return { { 0, yCenter(parent, child) }, child };
    case Position::Center:
      return { { xCenter(parent, child), yCenter(parent, child) }, child };
  }
}

constexpr Qt::Alignment
calculate(Position pos) noexcept
{
  Qt::Alignment align;
  switch (pos) {
    case Position::TopLeft:
    case Position::TopCenter:
    case Position::TopRight:
      align.setFlag(Qt::AlignmentFlag::AlignTop);
      break;
    case Position::CenterLeft:
    case Position::Center:
    case Position::CenterRight:
      align.setFlag(Qt::AlignmentFlag::AlignVCenter);
      break;
    case Position::BottomLeft:
    case Position::BottomCenter:
    case Position::BottomRight:
      align.setFlag(Qt::AlignmentFlag::AlignBottom);
      break;
    default:
      break;
  }
  switch (pos) {
    case Position::TopLeft:
    case Position::CenterLeft:
    case Position::BottomLeft:
      align.setFlag(Qt::AlignmentFlag::AlignLeft);
      break;
    case Position::TopCenter:
    case Position::Center:
    case Position::BottomCenter:
      align.setFlag(Qt::AlignmentFlag::AlignVCenter);
      break;
    case Position::TopRight:
    case Position::CenterRight:
    case Position::BottomRight:
      align.setFlag(Qt::AlignmentFlag::AlignRight);
      break;
    default:
      break;
  }
  return align;
}

QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size);
QRect
calculate(QWidget* widget, MiniWindow::Position position);
} // namespace geometry
