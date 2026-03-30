#pragma once
#include <QtWidgets/QAbstractButton>

class ColorPickerButton : public QAbstractButton
{
  Q_OBJECT
  Q_PROPERTY(QColor value READ value WRITE setValue NOTIFY valueChanged)
  Q_PROPERTY(bool alphaEnabled READ alphaEnabled WRITE setAlphaEnabled)
  Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)

public:
  explicit ColorPickerButton(QWidget* parent = nullptr);

  bool alphaEnabled() const noexcept { return m_alphaEnabled; }
  bool readOnly() const noexcept { return m_readOnly; }
  const QColor& value() const noexcept { return m_value; }

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

public slots:
  void openColorPicker();
  void setAlphaDisabled(bool disabled = true);
  void setAlphaEnabled(bool enabled = true);
  void setReadOnly(bool readOnly = true);
  void setValue(const QColor& value);

signals:
  void valueChanged(const QColor& value);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  QColor m_value;
  bool m_alphaEnabled = true;
  bool m_readOnly = false;
};
