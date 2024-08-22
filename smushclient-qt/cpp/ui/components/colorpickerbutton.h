#ifndef COLORPICKERBUTTON_H
#define COLORPICKERBUTTON_H

#include <QtWidgets/QPushButton>

class ColorPickerButton : public QPushButton
{
  Q_OBJECT

  Q_PROPERTY(QColor value READ value WRITE setValue NOTIFY valueChanged)

public:
  explicit ColorPickerButton(QWidget *parent = nullptr);

  const QColor &value() const;
  void click();

public Q_SLOTS:
  void setValue(const QColor &val);

Q_SIGNALS:
  void valueChanged(const QColor &val);
};

#endif // COLORPICKERBUTTON_H
