#pragma once
#include <QtWidgets/QLabel>

class IconLabel : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
  Q_PROPERTY(QIcon::Mode mode READ mode WRITE setMode)
  Q_PROPERTY(QIcon::State state READ state WRITE setState)

public:
  explicit IconLabel(const QIcon& icon, QWidget* parent = nullptr);
  explicit IconLabel(const QIcon& icon,
                     QIcon::Mode mode,
                     QWidget* parent = nullptr);
  explicit IconLabel(const QIcon& icon,
                     QIcon::State state,
                     QWidget* parent = nullptr);
  explicit IconLabel(const QIcon& icon,
                     QIcon::Mode mode,
                     QIcon::State state,
                     QWidget* parent = nullptr);

  const QIcon& icon() const noexcept { return m_icon; }
  QIcon::Mode mode() const noexcept { return m_mode; }
  QIcon::State state() const noexcept { return m_state; }

  bool hasHeightForWidth() const override;
  int heightForWidth(int w) const override;
  QSize sizeHint() const override;

public slots:
  void setIcon(const QIcon& icon,
               QIcon::Mode mode = QIcon::Normal,
               QIcon::State state = QIcon::On);
  void setFixedIconHeight(int height);
  void setMode(QIcon::Mode mode);
  void setState(QIcon::State state);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  int widthForHeight(int h) const;

private:
  QIcon m_icon;
  QIcon::Mode m_mode;
  QIcon::State m_state;
  QSize maxSize;
};
