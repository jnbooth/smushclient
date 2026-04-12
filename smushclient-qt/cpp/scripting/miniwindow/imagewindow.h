#pragma once
#include "miniwindow.h"

class ImageWindow : public QWidget
{
  Q_OBJECT

public:
  ImageWindow(const QString& path,
              QPixmap&& pixmap,
              MiniWindow::Position position,
              QWidget* parent = nullptr);
  MiniWindow::Position position() const { return m_position; }
  const QString& path() const { return m_path; }
  void setPixmap(const QString& path, QPixmap&& pixmap);
  void setPosition(MiniWindow::Position position);

public slots:
  void onParentResize();

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  using Position = MiniWindow::Position;
  void updatePosition();

private:
  QString m_path;
  QPixmap m_pixmap;
  MiniWindow::Position m_position;
  QSizeF scale;
};
