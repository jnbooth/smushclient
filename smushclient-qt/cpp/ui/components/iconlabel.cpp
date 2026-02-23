#include "iconlabel.h"
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

// Public methods

IconLabel::IconLabel(const QIcon& icon,
                     QIcon::Mode mode,
                     QIcon::State state,
                     QWidget* parent)
  : QWidget(parent)
  , m_icon(icon)
  , m_mode(mode)
  , m_state(state)
  , maxSize(icon.availableSizes().constLast())
{
}

IconLabel::IconLabel(const QIcon& icon, QWidget* parent)
  : IconLabel(icon, QIcon::Mode::Normal, QIcon::State::On, parent)
{
}

IconLabel::IconLabel(const QIcon& icon, QIcon::Mode mode, QWidget* parent)
  : IconLabel(icon, mode, QIcon::State::On, parent)
{
}

IconLabel::IconLabel(const QIcon& icon, QIcon::State state, QWidget* parent)
  : IconLabel(icon, QIcon::Mode::Normal, state, parent)
{
}

void
IconLabel::setFixedIconHeight(int height)
{
  setFixedSize(widthForHeight(height), height);
}

// Public slots

void
IconLabel::setIcon(const QIcon& newIcon,
                   QIcon::Mode newMode,
                   QIcon::State newState)
{
  m_icon = newIcon;
  m_mode = newMode;
  m_state = newState;
  maxSize = m_icon.availableSizes().constLast();
}

void
IconLabel::setMode(QIcon::Mode mode)
{
  if (m_mode == mode) {
    return;
  }
  m_mode = mode;
}

void
IconLabel::setState(QIcon::State state)
{
  if (m_state == state) {
    return;
  }
  m_state = state;
}

// Public overrides

bool
IconLabel::hasHeightForWidth() const
{
  return true;
}

int
IconLabel::heightForWidth(int w) const
{
  return w * maxSize.height() / maxSize.width();
}

QSize
IconLabel::sizeHint() const
{
  const int lineHeight = fontMetrics().lineSpacing();
  return QSize(widthForHeight(lineHeight), lineHeight);
}

// Protected overrides

void
IconLabel::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setClipRegion(event->region());
  m_icon.paint(&painter, rect(), Qt::Alignment(), m_mode, m_state);
}

// Private methods

int
IconLabel::widthForHeight(int h) const
{
  return h * maxSize.width() / maxSize.height();
}
