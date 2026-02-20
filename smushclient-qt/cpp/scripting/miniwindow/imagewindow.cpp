#include "imagewindow.h"
#include "geometry.h"

// Public methods

ImageWindow::ImageWindow(QPixmap&& pixmap, Position position, QWidget* parent)
  : QWidget(parent)
  , pixmap(std::move(pixmap))
  , position(position)
{
  updatePosition();
}

void
ImageWindow::setPosition(Position pos)
{
  if (pos == position) {
    return;
  }
  position = pos;
  updatePosition();
}

void
ImageWindow::setPixmap(QPixmap&& other)
{
  pixmap = std::move(other);
}

// Public slots

void
ImageWindow::onParentResize()
{
  if (position == Position::OutputScale ||
      position == Position::OutputStretch || position == Position::OwnerScale ||
      position == Position::OwnerStretch) {
    setGeometry(geometry::calculate(this, position, pixmap.size()));
  }
}

// Protected overrides

void
ImageWindow::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setClipRegion(event->region());
  if (position == Position::Tile) [[unlikely]] {
    painter.drawTiledPixmap(rect(), pixmap);
    return;
  }
  const QRect& rect = event->rect();
  if (scale.isNull()) {
    painter.drawPixmap(rect, pixmap, rect);
  } else {
    painter.drawPixmap(rect, pixmap, geometry::scale(rect, scale));
  }
}

void
ImageWindow::resizeEvent(QResizeEvent* event)
{
  switch (position) {
    case Position::OutputStretch:
    case Position::OwnerStretch: {
      const QSizeF thisSize = event->size().toSizeF();
      const QSizeF pixmapSize = pixmap.size().toSizeF();
      scale = QSizeF(pixmapSize.width() / thisSize.width(),
                     pixmapSize.height() / thisSize.height());
      break;
    }
    case Position::OwnerScale:
    case Position::OutputScale: {
      const QSizeF thisSize = event->size().toSizeF();
      const QSizeF pixmapSize = pixmap.size().toSizeF();
      const qreal heightScale = pixmapSize.height() / thisSize.height();
      scale = QSizeF(heightScale, heightScale);
      break;
    }
    default:
      break;
  }
}

// Private methods

void
ImageWindow::updatePosition()
{
  setGeometry(geometry::calculate(this, position, pixmap.size()));
}
