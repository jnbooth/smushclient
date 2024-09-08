#include "colorpickerbutton.h"
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtWidgets/QColorDialog>

ColorPickerButton::ColorPickerButton(QWidget *parent)
    : QAbstractButton(parent)
{
  connect(this, &QAbstractButton::clicked, this, &ColorPickerButton::openColorPicker);
}

// Public overrides

QSize ColorPickerButton::minimumSizeHint() const
{
  return minimumSize();
}

QSize ColorPickerButton::sizeHint() const
{
  return maximumSize();
}

// Public methods

void ColorPickerButton::openColorPicker()
{
  const QColor color = QColorDialog::getColor(currentValue, this, tr("Select Color"));
  if (!color.isValid())
    return;

  setValue(color);
}

void ColorPickerButton::setValue(const QColor &val)
{
  if (currentValue == val)
    return;

  currentValue = QColor(val);
  emit valueChanged(currentValue);
}

const QColor &ColorPickerButton::value() const &
{
  return currentValue;
}

// Protected overrides

void ColorPickerButton::paintEvent(QPaintEvent *)
{
  static const QPen borderDownPen(QBrush(Qt::GlobalColor::white), 1);
  static const QPen borderUpPen(QBrush(Qt::GlobalColor::black), 1);
  static const QPen borderFocusedPen(QBrush(Qt::GlobalColor::darkBlue), 1);

  QPainter painter(this);
  const QRect bounds = rect();
  painter.fillRect(bounds, currentValue);
  const QPen &pen = isDown()     ? borderDownPen
                    : hasFocus() ? borderFocusedPen
                                 : borderUpPen;
  painter.setPen(pen);
  painter.drawRect(bounds);
}
