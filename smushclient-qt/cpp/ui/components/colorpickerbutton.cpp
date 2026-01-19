#include "colorpickerbutton.h"
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
    currentValue,
    this,
    tr("Select Color"),
    alphaEnabled ? QColorDialog::ColorDialogOption::ShowAlphaChannel
                 : QColorDialog::ColorDialogOptions());

  if (!color.isValid()) {
    return;
  }

  setValue(color);
}

void
ColorPickerButton::setAlphaDisabled(bool disabled)
{
  alphaEnabled = !disabled;
}

void
ColorPickerButton::setAlphaEnabled(bool enabled)
{
  alphaEnabled = enabled;
}

void
ColorPickerButton::setValue(const QColor& val)
{
  if (currentValue == val) {
    return;
  }

  currentValue = QColor(val);
  emit valueChanged(currentValue);
  update();
}

const QColor&
ColorPickerButton::value() const&
{
  return currentValue;
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
  painter.fillRect(eventRect, currentValue);
  const QPen& pen = isDown()     ? borderDownPen
                    : hasFocus() ? borderFocusedPen
                                 : borderUpPen;
  painter.setPen(pen);
  painter.drawRect(rect());
}
