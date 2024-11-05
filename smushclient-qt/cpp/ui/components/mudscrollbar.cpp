#include "mudscrollbar.h"
#include <QtWidgets/QAbstractScrollArea>

// Private utils

QAbstractScrollArea *getScrollArea(const QObject *obj)
{
  if (!obj)
    return nullptr;

  QObject *parent = obj->parent();
  if (!parent)
    return nullptr;

  QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(parent);
  if (area)
    return area;

  return getScrollArea(parent);
}

// Public methods

MudScrollBar::MudScrollBar(QWidget *parent)
    : QScrollBar(parent),
      lastValue(0),
      isPaused(false),
      pausingEnabled(true) {}

void MudScrollBar::setPaused(bool paused)
{
  isPaused = paused;
  if (!isPaused)
    setValue(maximum());
}

void MudScrollBar::setPausingEnabled(bool enabled)
{
  pausingEnabled = enabled;
  if (pausingEnabled)
  {
    lastValue = value();
    isPaused = lastValue != maximum();
  }
  else
    isPaused = false;
  updateParentPolicy();
}

void MudScrollBar::toEnd()
{
  if (!isPaused)
    setValue(maximum());
}

// Protected overrides

void MudScrollBar::sliderChange(QAbstractSlider::SliderChange change)
{
  if (!pausingEnabled || change != QAbstractSlider::SliderChange::SliderValueChange)
    return;
  const int previousValue = lastValue;
  lastValue = value();
  if (lastValue == maximum())
  {
    if (!isPaused)
      return;
    isPaused = false;
  }
  else if (lastValue < previousValue)
  {
    if (isPaused)
      return;
    isPaused = true;
  }
  updateParentPolicy();
}

// Private methods

void MudScrollBar::updateParentPolicy() const
{
  QAbstractScrollArea *p = getScrollArea(parent());
  if (!p)
    return;

  p->setVerticalScrollBarPolicy(
      !pausingEnabled ? Qt::ScrollBarPolicy::ScrollBarAsNeeded
      : isPaused      ? Qt::ScrollBarPolicy::ScrollBarAlwaysOn
                      : Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}
