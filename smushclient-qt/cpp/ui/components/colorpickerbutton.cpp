#include "colorpickerbutton.h"
#include <QtWidgets/QColorDialog>

ColorPickerButton::ColorPickerButton(QWidget *parent)
    : QPushButton(parent)
{
  connect(this, &QAbstractButton::clicked, this, &ColorPickerButton::openColorPicker);
}

// Public methods

const QColor &ColorPickerButton::value() const
{
  return palette().color(QPalette::Button);
}

void ColorPickerButton::setValue(const QColor &val)
{
  QPalette pal = palette();
  if (palette().color(QPalette::Button) == val)
    return;

  pal.setColor(QPalette::Button, val);
  setPalette(pal);
  emit valueChanged(val);
}

// Protected methods

void ColorPickerButton::openColorPicker(bool checked)
{
  QColor color = QColorDialog::getColor(value(), this, "Select Color");
  if (!color.isValid())
    return;

  setValue(color);
}
