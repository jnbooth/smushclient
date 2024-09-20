#include "miniwindow.h"
#include <QtGui/QPainter>
#include <QtWidgets/QLayout>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include "hotspot.h"

using std::string;
using std::string_view;
using std::unordered_map;

// Utils

constexpr int xCenter(const QSize &parent, const QSize &child) noexcept
{
  return parent.height() - child.height() / 2;
}

constexpr int xRight(const QSize &parent, const QSize &child) noexcept
{
  return parent.height() - child.height();
}

constexpr int yCenter(const QSize &parent, const QSize &child) noexcept
{
  return parent.width() - child.width() / 2;
}

constexpr int yBottom(const QSize &parent, const QSize &child) noexcept
{
  return parent.height() - child.height();
}

QSize scaleWithAspectRatio(const QSize &parent, const QSize &child) noexcept
{
  const int height = parent.height();
  const int width = height * child.width() / child.height();
  return QSize(height, width);
}

constexpr QRect calculateGeometry(MiniWindow::Position pos, const QSize &parent, const QSize &child) noexcept
{
  switch (pos)
  {
  case MiniWindow::Position::OutputStretch:
  case MiniWindow::Position::OwnerStretch:
  case MiniWindow::Position::Tile:
    return QRect(QPoint(), parent);
  case MiniWindow::Position::OutputScale:
  case MiniWindow::Position::OwnerScale:
    return QRect(QPoint(), scaleWithAspectRatio(parent, child));
  case MiniWindow::Position::TopLeft:
    return QRect(QPoint(), child);
  case MiniWindow::Position::TopCenter:
    return QRect(QPoint(xCenter(parent, child), 0), child);
  case MiniWindow::Position::TopRight:
    return QRect(QPoint(xRight(parent, child), 0), child);
  case MiniWindow::Position::CenterRight:
    return QRect(QPoint(xRight(parent, child), yCenter(parent, child)), child);
  case MiniWindow::Position::BottomRight:
    return QRect(QPoint(xRight(parent, child), yBottom(parent, child)), child);
  case MiniWindow::Position::BottomCenter:
    return QRect(QPoint(xCenter(parent, child), yBottom(parent, child)), child);
  case MiniWindow::Position::BottomLeft:
    return QRect(QPoint(0, yBottom(parent, child)), child);
  case MiniWindow::Position::CenterLeft:
    return QRect(QPoint(0, yCenter(parent, child)), child);
  case MiniWindow::Position::Center:
    return QRect(QPoint(xCenter(parent, child), yCenter(parent, child)), child);
  }
}

Hotspot *findHotspotNeighbor(string_view hotspotID, const unordered_map<string, Hotspot *> hotspots)
{
  Hotspot *neighbor;
  string_view neighborID;
  for (const auto &entry : hotspots)
  {
    const string_view entryID = (string_view)entry.first;
    if (entryID == hotspotID)
      return entry.second;
    if (entryID > hotspotID && entryID < neighborID)
    {
      neighbor = entry.second;
      neighborID = entryID;
    }
  }
  return neighbor;
}

// Constructor

MiniWindow::MiniWindow(
    QWidget *parent,
    const QPoint &location,
    const QSize &size,
    Position position,
    Flags flags,
    const QColor &fill)
    : QWidget(parent),
      background(fill),
      dimensions(size),
      flags(flags),
      location(location),
      pixmap(size),
      position(position)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  pixmap.fill(background);
  applyFlags();
}

// Public methods

Hotspot *MiniWindow::addHotspot(
    string_view hotspotID,
    const Plugin *plugin,
    Hotspot::Callbacks &&callbacks)
{
  Hotspot *neighbor;
  string_view neighborID;
  for (const auto &entry : hotspots)
  {
    const string_view entryID = (string_view)entry.first;
    if (entryID == hotspotID)
    {
      Hotspot *hotspot = entry.second;
      if (!hotspot->belongsToPlugin(plugin))
        return nullptr;
      hotspot->setCallbacks(std::move(callbacks));
      return hotspot;
    }
    if (entryID > hotspotID && (!neighbor || entryID < neighborID))
    {
      neighbor = entry.second;
      neighborID = entryID;
    }
  }

  Hotspot *hotspot = new Hotspot(this, plugin, hotspotID, std::move(callbacks));
  hotspots[(string)hotspotID] = hotspot;
  if (neighbor)
    hotspot->stackUnder(neighbor);
  return hotspot;
}

void MiniWindow::clearHotspots()
{
  for (const auto &entry : hotspots)
    delete entry.second;
  hotspots.clear();
}

bool MiniWindow::deleteHotspot(string_view hotspotID)
{
  auto search = hotspots.find((string)hotspotID);
  if (search == hotspots.end())
    return false;
  delete search->second;
  hotspots.erase(search);
  return true;
}

void MiniWindow::drawLine(const QLineF &line, const QPen &pen)
{
  QPainter painter(&pixmap);
  painter.setPen(pen);
  painter.drawLine(line);
  updateMask();
  update();
}

void MiniWindow::drawEllipse(const QRectF &rect, const QPen &pen, const QBrush &brush)
{
  QPainter painter(&pixmap);
  painter.setPen(pen);
  painter.setBrush(brush);
  painter.drawEllipse(rect);
  updateMask();
  update();
}

void MiniWindow::drawFrame(const QRectF &rect, const QColor &color1, const QColor &color2)
{
  QPainter painter(&pixmap);
  painter.setPen(color1);
  painter.drawLine(rect.bottomLeft(), rect.topLeft());
  painter.drawLine(rect.topLeft(), rect.topRight());
  painter.setPen(color2);
  painter.drawLine(rect.topRight(), rect.bottomRight());
  painter.drawLine(rect.bottomRight(), rect.bottomLeft());
  updateMask();
  update();
}

void MiniWindow::drawPolygon(
    const QPolygonF &polygon,
    const QPen &pen,
    const QBrush &brush,
    Qt::FillRule fillRule)
{
  QPainter painter(&pixmap);
  painter.setPen(pen);
  painter.setBrush(brush);
  painter.drawPolygon(polygon, fillRule);
}

void MiniWindow::drawPolyline(const QPolygonF &polygon, const QPen &pen)
{
  QPainter painter(&pixmap);
  painter.setPen(pen);
  painter.drawPolyline(polygon);
}

void MiniWindow::drawRect(const QRectF &rect, const QPen &pen, const QBrush &brush)
{
  QPainter painter(&pixmap);
  painter.setPen(pen);
  painter.setBrush(brush);
  painter.drawRect(rect);
  updateMask();
  update();
}

void MiniWindow::drawRoundedRect(
    const QRectF &rect,
    qreal xRadius,
    qreal yRadius,
    const QPen &pen,
    const QBrush &brush)
{
  QPainter painter(&pixmap);
  painter.setPen(pen);
  painter.setBrush(brush);
  painter.drawRoundedRect(rect, xRadius, yRadius);
  updateMask();
  update();
}

QRectF MiniWindow::drawText(
    const QFont &font,
    const QString &text,
    const QRectF &rect,
    const QColor &color)
{
  QPainter painter(&pixmap);
  painter.setFont(font);
  painter.setPen(color);
  QRectF boundingRect;
  painter.drawText(rect, 0, text, &boundingRect);
  updateMask();
  update();
  return boundingRect;
}

Hotspot *MiniWindow::findHotspot(string_view hotspotID) const
{
  auto search = hotspots.find((string)hotspotID);
  if (search == hotspots.end())
    return nullptr;
  return search->second;
}

const QFont *MiniWindow::getFont(string_view fontID) const
{
  auto search = fonts.find((string)fontID);
  if (search == fonts.end())
    return nullptr;
  return &search->second;
}

int MiniWindow::getZOrder() const noexcept
{
  return zOrder;
}

void MiniWindow::reset()
{
  if (!flags.testFlag(Flag::KeepHotspots))
    clearHotspots();
}

void MiniWindow::setPosition(const QPoint &loc, Position pos, Flags newFlags) noexcept
{
  location = loc;
  position = pos;
  flags = newFlags;
  applyFlags();
}

void MiniWindow::setSize(const QSize &size, const QColor &fill) noexcept
{
  background = fill;
  dimensions = size;
}

void MiniWindow::setZOrder(int order) noexcept
{
  zOrder = order;
}

inline QSize getParentSize(const QWidget &widget)
{
  const QWidget *parent = widget.parentWidget();
  return parent ? parent->size() : widget.size();
}

void MiniWindow::updatePosition()
{
  const QRect geometry =
      flags.testFlag(Flag::Absolute)
          ? QRect(location, dimensions)
          : calculateGeometry(position, getParentSize(*this), dimensions);

  const QSize newDimensions = geometry.size();
  if (newDimensions != dimensions)
  {
    QPixmap newPixmap(newDimensions);
    newPixmap.fill(background);
    QPainter(&newPixmap).drawPixmap(QPointF(), pixmap);
    pixmap.swap(newPixmap);
  }

  setGeometry(geometry);
  updateMask();
}

// Protected overrides

void MiniWindow::paintEvent(QPaintEvent *event)
{
  if (position == Position::Tile) [[unlikely]]
  {
    QPainter(this).drawTiledPixmap(event->rect(), pixmap);
    return;
  }
  QPainter(this).drawPixmap(event->rect(), pixmap);
}

// Private methods

void MiniWindow::applyFlags()
{
  setAttribute(
      Qt::WA_TransparentForMouseEvents,
      flags.testAnyFlags(Flag::DrawUnderneath | Flag::IgnoreMouse));

  if (flags.testFlag(Flag::Transparent))
    updateMask();
  else
    clearMask();
}

inline void MiniWindow::updateMask()
{
  if (flags.testFlag(Flag::Transparent)) [[unlikely]]
    setMask(pixmap.createMaskFromColor(background));
}
