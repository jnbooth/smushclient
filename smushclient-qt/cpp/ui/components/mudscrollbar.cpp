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
      paused(false),
      pausingEnabled(true) {}

void MudScrollBar::setPausingEnabled(bool enabled)
{
  pausingEnabled = enabled;
  QAbstractScrollArea *p = getScrollArea(parent());
  if (!pausingEnabled)
  {
    paused = false;
    if (p)
      p->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    return;
  }
  lastValue = value();
  paused = lastValue != maximum();
  if (p)
    p->setVerticalScrollBarPolicy(
        paused ? Qt::ScrollBarPolicy::ScrollBarAlwaysOn
               : Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}

void MudScrollBar::toEnd()
{
  if (!paused)
    setValue(maximum());
}

void MudScrollBar::unpause()
{
  paused = false;
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
    if (!paused)
      return;
    paused = false;
  }
  else if (lastValue < previousValue)
  {
    if (paused)
      return;
    paused = true;
  }
  QAbstractScrollArea *p = getScrollArea(parent());
  if (!p)
    return;

  p->setVerticalScrollBarPolicy(
      paused ? Qt::ScrollBarPolicy::ScrollBarAlwaysOn
             : Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}
