#ifndef COLORPICKERBUTTON_H
#define COLORPICKERBUTTON_H

#include <QtWidgets/QAbstractButton>

class ColorPickerButton : public QAbstractButton
{
  Q_OBJECT

  Q_PROPERTY(QColor value READ value WRITE setValue NOTIFY valueChanged)

public:
  explicit ColorPickerButton(QWidget *parent = nullptr);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

  const QColor &value() const;

public slots:
  void openColorPicker();
  void setValue(const QColor &val);

signals:
  void valueChanged(const QColor &val);

protected:
  void paintEvent(QPaintEvent *) override;

protected slots:
  void onClicked(bool checked = false);

private:
  QColor currentValue;
};

#endif // COLORPICKERBUTTON_H
