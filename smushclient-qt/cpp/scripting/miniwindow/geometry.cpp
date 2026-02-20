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
