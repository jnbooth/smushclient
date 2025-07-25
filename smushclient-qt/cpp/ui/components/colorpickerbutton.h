#pragma once
#include <QtGui/QPaintEvent>
#include <QtWidgets/QAbstractButton>

class ColorPickerButton : public QAbstractButton {
  Q_OBJECT

  Q_PROPERTY(QColor value READ value WRITE setValue NOTIFY valueChanged)

public:
  explicit ColorPickerButton(QWidget *parent = nullptr);
  virtual ~ColorPickerButton() {};

  constexpr bool alphaEnabled() const noexcept { return isAlphaEnabled; }
  const QColor &value() const &;

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

public slots:
  void openColorPicker();
  void setAlphaDisabled(bool disabled = true);
  void setAlphaEnabled(bool enabled = true);
  void setValue(const QColor &value);

signals:
  void valueChanged(const QColor &value);

protected:
  virtual void paintEvent(QPaintEvent *event) override;

private:
  QColor currentValue{};
  bool isAlphaEnabled = true;
};
