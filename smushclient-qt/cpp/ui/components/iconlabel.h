#pragma once
#include <QtGui/QIcon>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QLabel>

class IconLabel : public QLabel
{
  Q_OBJECT

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

  void setIcon(const QIcon& icon,
               QIcon::Mode mode = QIcon::Normal,
               QIcon::State state = QIcon::On);
  void setFixedIconHeight(int height);
  void setMode(QIcon::Mode mode);
  void setState(QIcon::State state);

  bool hasHeightForWidth() const override;
  int heightForWidth(int w) const override;
  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  int widthForHeight(int h) const;

private:
  QIcon icon;
  QSize maxSize;
  QIcon::Mode mode;
  bool needsUpdate = true;
  QIcon::State state;
};
