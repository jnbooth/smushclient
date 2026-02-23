#include "colorpickerbutton.h"
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtWidgets/QColorDialog>

// Public methods

ColorPickerButton::ColorPickerButton(QWidget* parent)
  : QAbstractButton(parent)
{
  connect(
    this, &QAbstractButton::clicked, this, &ColorPickerButton::openColorPicker);
}

void
ColorPickerButton::openColorPicker()
{
  const QColor color = QColorDialog::getColor(
    value(),
    this,
    tr("Select Color"),
    alphaEnabled() ? QColorDialog::ColorDialogOption::ShowAlphaChannel
                   : QColorDialog::ColorDialogOptions());

  if (!color.isValid()) {
    return;
  }

  setValue(color);
}

void
ColorPickerButton::setAlphaDisabled(bool disabled)
{
  m_alphaEnabled = !disabled;
}

void
ColorPickerButton::setAlphaEnabled(bool enabled)
{
  m_alphaEnabled = enabled;
}

void
ColorPickerButton::setValue(const QColor& value)
{
  if (m_value == value) {
    return;
  }

  m_value = QColor(value);
  emit valueChanged(m_value);
  update();
}

// Public overrides

QSize
ColorPickerButton::minimumSizeHint() const
{
  return minimumSize();
}

QSize
ColorPickerButton::sizeHint() const
{
  return baseSize();
}

// Protected overrides

void
ColorPickerButton::paintEvent(QPaintEvent* event)
{
  static const QPen borderDownPen(Qt::GlobalColor::white, 1);
  static const QPen borderUpPen(Qt::GlobalColor::black, 1);
  static const QPen borderFocusedPen(Qt::GlobalColor::darkBlue, 1);

  const QRect& eventRect = event->rect();

  QPainter painter(this);
  painter.setClipRegion(event->region());
  painter.fillRect(eventRect, value());
  const QPen& pen = isDown()     ? borderDownPen
                    : hasFocus() ? borderFocusedPen
                                 : borderUpPen;
  painter.setPen(pen);
  painter.drawRect(rect());
}
