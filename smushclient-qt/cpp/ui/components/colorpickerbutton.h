#pragma once
#include <QtWidgets/QAbstractButton>

class ColorPickerButton : public QAbstractButton
{
  Q_OBJECT
  Q_PROPERTY(QColor value READ value WRITE setValue NOTIFY valueChanged)
  Q_PROPERTY(bool alphaEnabled READ alphaEnabled WRITE setAlphaEnabled)

public:
  explicit ColorPickerButton(QWidget* parent = nullptr);

  bool alphaEnabled() const noexcept { return m_alphaEnabled; }
  const QColor& value() const noexcept { return m_value; }

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

public slots:
  void openColorPicker();
  void setAlphaDisabled(bool disabled = true);
  void setAlphaEnabled(bool enabled = true);
  void setValue(const QColor& value);

signals:
  void valueChanged(const QColor& value);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  bool m_alphaEnabled = true;
  QColor m_value;
};
