#pragma once
#include <QtGui/QIcon>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QLabel>

class IconLabel : public QLabel
{
  Q_OBJECT

public:
  explicit IconLabel(const QIcon &icon, QWidget *parent = nullptr);
  explicit IconLabel(const QIcon &icon, QIcon::Mode mode, QWidget *parent = nullptr);
  explicit IconLabel(const QIcon &icon, QIcon::State state, QWidget *parent = nullptr);
  explicit IconLabel(const QIcon &icon, QIcon::Mode mode, QIcon::State state, QWidget *parent = nullptr);
  virtual ~IconLabel() {}

  void setIcon(const QIcon &icon, QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::On);
  void setFixedIconHeight(int height);
  void setMode(QIcon::Mode mode);
  void setState(QIcon::State state);

  virtual bool hasHeightForWidth() const override;
  virtual int heightForWidth(int w) const override;
  virtual QSize sizeHint() const override;

protected:
  virtual void paintEvent(QPaintEvent *event) override;
  virtual void resizeEvent(QResizeEvent *event) override;

private:
  QIcon icon;
  QSize maxSize;
  QIcon::Mode mode;
  bool needsUpdate;
  QIcon::State state;

private:
  int widthForHeight(int h) const;
};
