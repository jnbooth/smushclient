#include "geometry.h"

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
QRect
calculate(QWidget* widget, MiniWindow::Position position, const QSize& size)
{
  return calculate(position, getParentWidget(widget)->size(), size);
}

QRect
calculate(QWidget* widget, MiniWindow::Position position)
{
  return calculate(widget, position, widget->size());
}
} // namespace geometry
