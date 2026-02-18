#pragma once
#include "miniwindow.h"

class ImageWindow : public QWidget
{
  Q_OBJECT

public:
  ImageWindow(QPixmap&& pixmap,
              MiniWindow::Position position,
              QWidget* parent = nullptr);
  void setPixmap(QPixmap&& other);
  void setPosition(MiniWindow::Position pos);

public slots:
  void onParentResize();

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  using Position = MiniWindow::Position;
  void updatePosition(const QSize& size);

private:
  QPixmap pixmap;
  MiniWindow::Position position;
  QSizeF scale;
};
