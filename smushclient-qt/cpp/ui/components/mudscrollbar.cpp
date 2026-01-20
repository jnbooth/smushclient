#include "mudscrollbar.h"
#include <QtWidgets/QAbstractScrollArea>

// Private utils

namespace {
QAbstractScrollArea*
getScrollArea(const QObject* obj)
{
  if (obj == nullptr) {
    return nullptr;
  }

  QObject* parent = obj->parent();
  if (parent == nullptr) {
    return nullptr;
  }

  QAbstractScrollArea* area = qobject_cast<QAbstractScrollArea*>(parent);
  if (area != nullptr) {
    return area;
  }

  return getScrollArea(parent);
}
} // namespace

// Public methods

MudScrollBar::MudScrollBar(QWidget* parent)
  : QScrollBar(parent)
{
}

// Public slots

void
MudScrollBar::setAutoScrollEnabled(bool enabled)
{
  autoScroll = enabled;
  if (autoScroll && !paused) {
    setValue(maximum());
  }
}

void
MudScrollBar::setPaused(bool isPaused)
{
  paused = isPaused;
  if (!paused) {
    setValue(maximum());
  }
}

void
MudScrollBar::setPausingEnabled(bool enabled)
{
  pausingEnabled = enabled;
  if (pausingEnabled) {
    lastValue = value();
    paused = lastValue != maximum();
  } else {
    paused = false;
  }
  updateParentPolicy();
}

// Protected overrides

void
MudScrollBar::sliderChange(QAbstractSlider::SliderChange change)
{
  if (autoScroll && !paused &&
      change == QAbstractSlider::SliderChange::SliderRangeChange) {
    inInternalChange = true;
    setValue(maximum());
    return;
  }
  if (change != QAbstractSlider::SliderChange::SliderValueChange) {
    return;
  }
  if (inInternalChange) {
    inInternalChange = false;
    return;
  }
  if (!pausingEnabled) {
    return;
  }
  const int previousValue = lastValue;
  lastValue = value();
  if (lastValue == maximum()) {
    if (!paused) {
      return;
    }
    paused = false;
  } else if (lastValue < previousValue) {
    if (paused) {
      return;
    }
    paused = true;
  }
  updateParentPolicy();
}

// Private methods

void
MudScrollBar::updateParentPolicy() const
{
  QAbstractScrollArea* p = getScrollArea(parent());
  if (p == nullptr) {
    return;
  }

  p->setVerticalScrollBarPolicy(
    !pausingEnabled ? Qt::ScrollBarPolicy::ScrollBarAsNeeded
    : paused        ? Qt::ScrollBarPolicy::ScrollBarAlwaysOn
                    : Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}
