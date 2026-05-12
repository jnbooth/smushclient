#include "imagewindow.h"
#include "geometry.h"

// Public methods

ImageWindow::ImageWindow(const QString& path,
                         QPixmap&& pixmap,
                         Position position,
                         QWidget* parent)
  : QWidget(parent)
  , m_path(path)
  , m_pixmap(std::move(pixmap))
  , m_position(position)
{
  updatePosition();
}

void
ImageWindow::setPosition(Position position)
{
  if (position == m_position) {
    return;
  }
  m_position = position;
  updatePosition();
}

void
ImageWindow::setPixmap(const QString& path, QPixmap&& pixmap)
{
  m_path = path;
  m_pixmap = std::move(pixmap);
}

// Public slots

void
ImageWindow::onParentResize()
{
  if (m_position == Position::OutputScale ||
      m_position == Position::OutputStretch ||
      m_position == Position::OwnerScale ||
      m_position == Position::OwnerStretch) {
    setGeometry(geometry::calculate(this, m_position, m_pixmap.size()));
  }
}

// Protected overrides

void
ImageWindow::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setClipRegion(event->region());
  if (m_position == Position::Tile) [[unlikely]] {
    painter.drawTiledPixmap(rect(), m_pixmap);
    return;
  }
  const QRect& rect = event->rect();
  if (scale.isNull()) {
    painter.drawPixmap(rect, m_pixmap, rect);
  } else {
    painter.drawPixmap(rect, m_pixmap, geometry::scale(rect, scale));
  }
}

void
ImageWindow::resizeEvent(QResizeEvent* event)
{
  switch (m_position) {
    case Position::OutputStretch:
    case Position::OwnerStretch: {
      const QSize thisSize = event->size();
      const QSize pixmapSize = m_pixmap.size();
      if (thisSize == pixmapSize) {
        scale = QSizeF{};
        break;
      }
      const QSizeF thisSizeF = thisSize;
      const QSizeF pixmapSizeF = pixmapSize;
      scale = { pixmapSizeF.width() / thisSizeF.width(),
                pixmapSizeF.height() / thisSizeF.height() };
      break;
    }
    case Position::OwnerScale:
    case Position::OutputScale: {
      const int thisHeight = event->size().height();
      const int pixmapHeight = m_pixmap.height();
      if (thisHeight == pixmapHeight) {
        scale = QSizeF{};
        break;
      }
      const qreal thisHeightF = thisHeight;
      const qreal pixmapHeightF = pixmapHeight;
      const qreal heightScale = thisHeightF / pixmapHeightF;
      scale = { heightScale, heightScale };
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
  setGeometry(geometry::calculate(this, m_position, m_pixmap.size()));
}
