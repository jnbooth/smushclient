#include "geometry.h"
#include <numbers>

// Private utils

namespace {
const QWidget*
getParentWidget(const QWidget* widget)
{
  if (widget == nullptr) [[unlikely]] {
    return widget;
  }
  const QWidget* parent = widget->parentWidget();
  if (parent == nullptr) [[unlikely]] {
    return widget;
  }
  return parent;
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
} // namespace

// Public functions

namespace geometry {
qreal
arc(const QPointF& center, const QPointF& edge)
{
  if (center == edge) {
    return NAN;
  }
  const qreal xDist = edge.x() - center.x();
  const qreal yDist = edge.y() - center.y();
  if (xDist == 0) {
    return yDist >= 0 ? 90.0 * 16.0 : 270.0 * 16.0;
  }
  const qreal arc = std::atan(yDist / xDist) * 180.0 * 16.0 / std::numbers::pi;
  if (xDist < 0) {
    return arc + 180.0 * 16.0;
  }
  if (arc < 0) {
    return arc + 360.0 * 16.0;
  }
  return arc;
}

QRect
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

QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size)
{
  const QWidget* parent = getParentWidget(widget);
  if (position == MiniWindow::Position::OwnerScale ||
      position == MiniWindow::Position::OwnerStretch) {
    parent = getParentWidget(parent);
  }
  return calculate(position, parent->size(), size);
}

QRect
calculate(QWidget* widget, MiniWindow::Position position)
{
  return calculate(widget, position, widget->size());
}
} // namespace geometry
