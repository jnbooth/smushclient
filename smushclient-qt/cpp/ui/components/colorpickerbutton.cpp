#include "colorpickerbutton.h"
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtWidgets/QColorDialog>

QPen borderDownPen(QBrush(Qt::GlobalColor::white), 1);
QPen borderUpPen(QBrush(Qt::GlobalColor::black), 1);
QPen borderFocusedPen(QBrush(Qt::GlobalColor::darkBlue), 1);

ColorPickerButton::ColorPickerButton(QWidget *parent) : QAbstractButton(parent)
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
  QColor color = QColorDialog::getColor(currentValue, this, "Select Color");
  if (!color.isValid())
    return;

  setValue(color);
}

void ColorPickerButton::setValue(const QColor &val)
{
  if (currentValue == val)
    return;

  currentValue = val;
  emit valueChanged(val);
}

const QColor &ColorPickerButton::value() const
{
  return currentValue;
}

// Protected overrides

void ColorPickerButton::paintEvent(QPaintEvent *e)
{
  QPainter painter(this);
  QRect rect = this->rect();
  painter.fillRect(rect, currentValue);
  QPen &pen = isDown()     ? borderDownPen
              : hasFocus() ? borderFocusedPen
                           : borderUpPen;
  painter.setPen(pen);
  painter.drawRect(rect);
}
