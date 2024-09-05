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

public slots:
  void setValue(const QColor &val);

signals:
  void valueChanged(const QColor &val);

protected slots:
  void openColorPicker(bool checked = false);
};

#endif // COLORPICKERBUTTON_H
