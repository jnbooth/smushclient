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
    : QScrollBar(parent) {}

// Public slots

void MudScrollBar::setAutoScrollEnabled(bool enabled)
{
  autoScroll = enabled;
  if (autoScroll && !isPaused)
    setValue(maximum());
}

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

// Protected overrides

void MudScrollBar::sliderChange(QAbstractSlider::SliderChange change)
{
  if (autoScroll && !isPaused && change == QAbstractSlider::SliderChange::SliderRangeChange)
  {
    inInternalChange = true;
    setValue(maximum());
    return;
  }
  if (change != QAbstractSlider::SliderChange::SliderValueChange)
    return;
  if (inInternalChange)
  {
    inInternalChange = false;
    return;
  }
  if (!pausingEnabled)
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
