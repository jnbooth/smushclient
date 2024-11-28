#include "iconlabel.h"

// Public methods

IconLabel::IconLabel(const QIcon &icon, QIcon::Mode mode, QIcon::State state, QWidget *parent)
    : QLabel(parent),
      icon(icon),
      maxSize(icon.availableSizes().constLast()),
      mode(mode),
      state(state) {}

IconLabel::IconLabel(const QIcon &icon, QWidget *parent)
    : IconLabel(icon, QIcon::Mode::Normal, QIcon::State::On, parent) {}

IconLabel::IconLabel(const QIcon &icon, QIcon::Mode mode, QWidget *parent)
    : IconLabel(icon, mode, QIcon::State::On, parent) {}

IconLabel::IconLabel(const QIcon &icon, QIcon::State state, QWidget *parent)
    : IconLabel(icon, QIcon::Mode::Normal, state, parent) {}

void IconLabel::setFixedIconHeight(int height)
{
  setFixedSize(widthForHeight(height), height);
}

void IconLabel::setIcon(const QIcon &newIcon, QIcon::Mode newMode, QIcon::State newState)
{
  icon = newIcon;
  mode = newMode;
  state = newState;
  maxSize = icon.availableSizes().constLast();
  needsUpdate = true;
}

void IconLabel::setMode(QIcon::Mode newMode)
{
  mode = newMode;
  needsUpdate = true;
}

void IconLabel::setState(QIcon::State newState)
{
  state = newState;
  needsUpdate = true;
}

// Public overrides

bool IconLabel::hasHeightForWidth() const
{
  return true;
}

int IconLabel::heightForWidth(int w) const
{
  return w * maxSize.height() / maxSize.width();
}

QSize IconLabel::sizeHint() const
{
  const int lineHeight = fontMetrics().lineSpacing();
  return QSize(widthForHeight(lineHeight), lineHeight);
}

// Protected overrides

void IconLabel::paintEvent(QPaintEvent *event)
{
  if (needsUpdate) [[unlikely]]
  {
    needsUpdate = false;
    setPixmap(icon.pixmap(size(), mode, state));
  }
  QLabel::paintEvent(event);
}

void IconLabel::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  needsUpdate = true;
}

// Private methods

int IconLabel::widthForHeight(int h) const
{
  return h * maxSize.width() / maxSize.height();
}
