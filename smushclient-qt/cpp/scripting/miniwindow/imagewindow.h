#pragma once
#include "miniwindow.h"

class ImageWindow : public QWidget
{
  Q_OBJECT

private:
  using Position = MiniWindow::Position;

public:
  ImageWindow(const QString& path,
              QPixmap&& pixmap,
              Position position,
              QWidget* parent = nullptr);
  Position position() const { return m_position; }
  const QString& path() const { return m_path; }
  void setPixmap(const QString& path, QPixmap&& pixmap);
  void setPosition(Position position);

public slots:
  void onParentResize();

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  void updatePosition();

private:
  QString m_path;
  QPixmap m_pixmap;
  MiniWindow::Position m_position;
  QSizeF scale;
};
