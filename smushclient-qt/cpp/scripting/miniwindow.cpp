#include "miniwindow.h"
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include "hotspot.h"

using std::string;
using std::string_view;

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

Hotspot *MiniWindow::addHotspot(string_view id, const Plugin *plugin, Hotspot::Callbacks &&callbacks)
{
  string hotspotID = (string)id;
  Hotspot *hotspot = hotspots[hotspotID];
  if (hotspot == nullptr)
    hotspot = hotspots[hotspotID] = new Hotspot(this, plugin, hotspotID, std::move(callbacks));
  else if (!hotspot->belongsToPlugin(plugin))
    return nullptr;
  else
    hotspot->setCallbacks(std::move(callbacks));
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

void MiniWindow::drawRect(const QRect &rect, const QColor &color)
{
  QPainter painter(&pixmap);
  painter.setBrush(color);
  painter.drawRect(rect);
  updateMask();
}

Hotspot *MiniWindow::findHotspot(string_view hotspotID) const
{
  auto search = hotspots.find((string)hotspotID);
  if (search == hotspots.end())
    return nullptr;
  return search->second;
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

void MiniWindow::paintEvent(QPaintEvent *)
{
  if (position == Position::Tile) [[unlikely]]
  {
    QPainter(this).drawTiledPixmap(rect(), pixmap);
    return;
  }
  QPainter(this).drawPixmap(QPointF(), pixmap);
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