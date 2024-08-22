#include "colorpickerbutton.h"
#include <QtWidgets/QColorDialog>

ColorPickerButton::ColorPickerButton(QWidget *parent)
    : QPushButton(parent)
{
  setFixedWidth(height());
}

const QColor &ColorPickerButton::value() const
{
  return palette().color(QPalette::ColorRole::Button);
}

void ColorPickerButton::setValue(const QColor &val)
{
  QPalette pal = palette();
  if (palette().color(QPalette::ColorRole::Button) == val)
  {
    return;
  }
  pal.setColor(QPalette::ColorRole::Button, val);
  setPalette(pal);
  emit valueChanged(val);
}

void ColorPickerButton::click()
{
  QColor color = QColorDialog::getColor(value(), this, "Select Color");
  if (!color.isValid())
  {
    return;
  }
  setValue(color);
}
