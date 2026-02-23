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
  if (m_autoScroll == enabled) {
    return;
  }
  m_autoScroll = enabled;
  if (m_autoScroll && !m_paused) {
    setValue(maximum());
  }
}

void
MudScrollBar::setPaused(bool paused)
{
  if (m_paused == paused) {
    return;
  }
  m_paused = paused;
  if (!m_paused) {
    setValue(maximum());
  }
}

void
MudScrollBar::setPausingEnabled(bool enabled)
{
  if (m_pausingEnabled == enabled) {
    return;
  }
  m_pausingEnabled = enabled;
  if (m_pausingEnabled) {
    lastValue = value();
    m_paused = lastValue != maximum();
  } else {
    m_paused = false;
  }
  updateParentPolicy();
}

// Protected overrides

void
MudScrollBar::sliderChange(QAbstractSlider::SliderChange change)
{
  if (m_autoScroll && !m_paused &&
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
  if (!m_pausingEnabled) {
    return;
  }
  const int previousValue = lastValue;
  lastValue = value();
  if (lastValue == maximum()) {
    if (!m_paused) {
      return;
    }
    m_paused = false;
  } else if (lastValue < previousValue) {
    if (m_paused) {
      return;
    }
    m_paused = true;
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
    !m_pausingEnabled ? Qt::ScrollBarPolicy::ScrollBarAsNeeded
    : m_paused        ? Qt::ScrollBarPolicy::ScrollBarAlwaysOn
                      : Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
}
