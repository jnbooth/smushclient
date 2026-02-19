#include "miniwindow.h"
#include "blend.h"
#include "geometry.h"
#include "hotspot.h"
#include "imagefilters.h"
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMenu>
#include <algorithm>
#include <cmath>

using std::span;
using std::string;
using std::string_view;
using std::vector;

// Private utils

namespace {
constexpr span<const char>
trim(const string& s) noexcept
{
  const size_t start = s.find_first_not_of(' ');
  const size_t end = s.find_last_not_of(' ');
  return span(s).subspan(start, end + 1 - start);
}

inline bool
buildMenu(QMenu& menu, string_view text)
{
  const size_t menuCount = 1 + std::count(text.cbegin(), text.cend(), '>');
  vector<QMenu*> menus;
  menus.reserve(menuCount);
  menus.push_back(&menu);
  std::istringstream stream((string(text)));
  const bool returnsNumber = stream.peek() == '!';
  if (returnsNumber) {
    stream.get();
  }
  if (stream.peek() == '~') {
    stream.get(); // ~
    stream.get(); // horizontal alignment
    stream.get(); // vertical alignment
  }
  for (string untrimmed; std::getline(stream, untrimmed, '|');) {
    const span<const char> item = trim(untrimmed);
    if (item.empty()) {
      menus.back()->addSeparator();
      continue;
    }
    const char first = item.front();
    switch (first) {
      case '-':
        if (item.size() == 1) {
          menus.back()->addSeparator();
          continue;
        }
        break;
      case '<': // un-nest
        if (menus.size() > 1) {
          menus.pop_back();
        }
        continue;
      case '>': // nested menu
      {
        const QString menuTitle = QString::fromUtf8(item.subspan(1));
        menus.push_back(menus.back()->addMenu(menuTitle));
        continue;
      }
      default:
        break;
    };
    QMenu* buildingMenu = menus.back();
    QAction* action = new QAction(buildingMenu);

    size_t offset = 0;
    for (const char& c : item) {
      switch (c) {
        case '+':
          ++offset;
          action->setCheckable(true);
          action->setChecked(true);
          continue;
        case '^':
          ++offset;
          action->setDisabled(true);
          continue;
        default:
          break;
      }
    }
    action->setText(QString::fromUtf8(item.subspan(offset)));
    buildingMenu->addAction(action);
  }
  return returnsNumber;
}
} // namespace

// Painter

MiniWindow::Painter::Painter(MiniWindow* window)
  : QPainter(&window->pixmap)
  , window(window)
{
}

MiniWindow::Painter::Painter(MiniWindow* window, CompositionMode mode)
  : MiniWindow::Painter(window)
{
  setCompositionMode(mode);
}

MiniWindow::Painter::Painter(MiniWindow* window, const QPen& pen)
  : MiniWindow::Painter(window)
{
  setPen(pen);
}

MiniWindow::Painter::Painter(MiniWindow* window,
                             const QPen& pen,
                             const QBrush& brush)
  : MiniWindow::Painter(window, pen)
{
  setBrush(brush);
}

MiniWindow::Painter::~Painter()
{
  window->updateMask();
  window->update();
}

// Public methods

MiniWindow::MiniWindow(const QPoint& location,
                       const QSize& size,
                       Position position,
                       Flags flags,
                       const QColor& fill,
                       string_view pluginID,
                       QWidget* parent)
  : QWidget(parent)
  , background(fill)
  , dimensions(size)
  , flags(flags)
  , installed(QDateTime::currentDateTime())
  , location(location)
  , pluginID(pluginID)
  , position(position)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  pixmap.setDevicePixelRatio(devicePixelRatio());
  applyFlags();
}

Hotspot*
MiniWindow::addHotspot(string_view hotspotID,
                       WorldTab& tab,
                       const Plugin& plugin,
                       Hotspot::Callbacks&& callbacks)
{
  Hotspot* neighbor = nullptr;
  string_view neighborID;
  for (const auto& entry : hotspots) {
    const string_view entryID = entry.first;
    if (entryID == hotspotID) {
      Hotspot* hotspot = &*entry.second;
      if (!hotspot->belongsToPlugin(plugin)) {
        return nullptr;
      }
      hotspot->setCallbacks(std::move(callbacks));
      return hotspot;
    }
    if (entryID < hotspotID &&
        ((neighbor == nullptr) || entryID > neighborID)) {
      neighbor = &*entry.second;
      neighborID = entryID;
    }
  }

  auto& hotspot = hotspots[string(hotspotID)] = std::make_unique<Hotspot>(
    tab, plugin, hotspotID, std::move(callbacks), this);
  if (neighbor != nullptr) {
    hotspot->stackUnder(neighbor);
  }
  return &*hotspot;
}

void
MiniWindow::applyFilter(const ImageFilter& filter, const QRect& rectBase)
{
  const QRect rect = normalize(rectBase);
  if (rectBase == pixmap.rect()) {
    pixmap.convertFromImage(filter.apply(pixmap));
    return;
  }
  QPixmap section = pixmap.copy(rect);
  Painter(this, QPainter::CompositionMode_Source)
    .drawImage(rect.topLeft(), filter.apply(section));
}

void
MiniWindow::blendImage(BlendMode mode,
                       const QPixmap& image,
                       const QRectF& rectBase,
                       qreal opacity,
                       const QRectF& sourceRectBase)
{

  QRectF rect = normalize(rectBase);
  QRectF sourceRect = geometry::normalize(sourceRectBase, image.size());
  const QSizeF size = rect.size().boundedTo(sourceRect.size());
  rect.setSize(size);
  sourceRect.setSize(size);
  blend::image(pixmap, image, rect.topLeft(), mode, opacity, sourceRect);
  updateMask();
  update();
}

void
MiniWindow::clearHotspots()
{
  hotspots.clear();
}

bool
MiniWindow::deleteHotspot(string_view hotspotID)
{
  auto search = hotspots.find(hotspotID);
  if (search == hotspots.end()) {
    return false;
  }
  hotspots.erase(search);
  return true;
}

void
MiniWindow::drawArc(const QRectF& rect,
                    const QPointF& start,
                    const QPointF& end,
                    const QPen& pen)
{
  const QPointF center = rect.center();
  const qreal startArc = geometry::arc(center, start);
  if (std::isnan(startArc)) {
    return;
  }
  const qreal endArc = geometry::arc(center, end);
  if (std::isnan(endArc)) {
    return;
  }
  Painter(this, pen).drawArc(
    rect, static_cast<int>(startArc), static_cast<int>(endArc));
}

void
MiniWindow::drawButton(const QRect& rectBase,
                       ButtonFrame frameType,
                       ButtonFlags buttonFlags)
{
  const QRect rect = normalize(rectBase);
  QFrame frame;
  frame.setFixedSize(rect.size());
  if (!buttonFlags.testFlag(ButtonFlag::Fill)) {
    frame.setAttribute(Qt::WA_NoSystemBackground);
    frame.setAttribute(Qt::WA_TranslucentBackground);
  }
  switch (frameType) {
    case ButtonFrame::Raised:
      frame.setFrameShape(QFrame::Shape::Panel);
      frame.setFrameShadow(QFrame::Shadow::Sunken);
      break;
    case ButtonFrame::Etched:
      frame.setFrameShape(QFrame::Shape::Box);
      frame.setFrameShadow(QFrame::Shadow::Raised);
      break;
    case ButtonFrame::Bump:
      frame.setFrameShape(QFrame::Shape::Box);
      frame.setFrameShadow(QFrame::Shadow::Sunken);
      break;
    case ButtonFrame::Sunken:
      frame.setFrameShape(QFrame::Shape::Panel);
      frame.setFrameShadow(QFrame::Shadow::Raised);
      break;
  }
  if (buttonFlags.testAnyFlags(
        ButtonFlags(ButtonFlag::Flat | ButtonFlag::Monochrome))) {
    frame.setFrameShadow(QFrame::Shadow::Plain);
  }

  if (buttonFlags.testFlag(ButtonFlag::Flat)) {
    frame.setLineWidth(1);
  } else if (buttonFlags.testFlag(ButtonFlag::Soft)) {
    frame.setLineWidth(2);
  } else {
    frame.setLineWidth(3);
  }

  Painter painter(this);
  frame.render(&painter, rect.topLeft());
}

void
MiniWindow::drawEllipse(const QRectF& rect,
                        const QPen& pen,
                        const QBrush& brush)
{
  Painter(this, pen, brush).drawEllipse(normalize(rect));
}

void
MiniWindow::drawFrame(const QRectF& rectBase,
                      const QColor& color1,
                      const QColor& color2)
{
  const QRectF rect = normalize(rectBase);
  Painter painter(this);
  painter.setPen(color1);
  painter.drawLine(rect.bottomLeft(), rect.topLeft());
  painter.drawLine(rect.topLeft(), rect.topRight());
  painter.setPen(color2);
  painter.drawLine(rect.topRight(), rect.bottomRight());
  painter.drawLine(rect.bottomRight(), rect.bottomLeft());
}

void
MiniWindow::drawGradient(const QRectF& rect, const QGradient& gradient)
{
  Painter(this).fillRect(rect, gradient);
}

void
MiniWindow::drawImage(const QPixmap& image,
                      const QRectF& rectBase,
                      const QRectF& sourceRectBase,
                      DrawImageMode mode,
                      qreal opacity)
{
  Painter painter(this);
  painter.setOpacity(opacity);
  const QRectF rect = normalize(rectBase);
  const QRectF sourceRect = geometry::normalize(sourceRectBase, image.size());
  switch (mode) {
    case DrawImageMode::Copy:
      painter.drawPixmap(rect.topLeft(), image, sourceRect);
      return;
    case DrawImageMode::Stretch:
      painter.drawPixmap(rect, image, sourceRect);
      return;
    case DrawImageMode::CopyTransparent:
      QPixmap croppedImage = image.copy(sourceRect.toRect());
      if (croppedImage.isNull()) {
        return;
      }
      const QImage qImage = image.copy(QRect(0, 0, 2, 2))
                              .toImage()
                              .convertToFormat(QImage::Format_RGB32);
      const QRgb pixel = qImage.pixel(0, 0);
      croppedImage.setMask(
        QBitmap::fromImage(std::move(qImage).createMaskFromColor(pixel)));
      Painter(this).drawPixmap(rect.topLeft(), croppedImage);
  }
}

void
MiniWindow::drawLine(const QLineF& line, const QPen& pen)
{
  Painter(this, pen).drawLine(line);
}

void
MiniWindow::drawPolygon(const QPolygonF& polygon,
                        const QPen& pen,
                        const QBrush& brush,
                        Qt::FillRule fillRule)
{
  Painter(this, pen, brush).drawPolygon(polygon, fillRule);
}

void
MiniWindow::drawPolyline(const QPolygonF& polygon, const QPen& pen)
{
  Painter(this, pen).drawPolyline(polygon);
}

void
MiniWindow::drawRect(const QRectF& rect, const QPen& pen, const QBrush& brush)
{
  Painter(this, pen, brush).drawRect(normalize(rect));
}

void
MiniWindow::drawRoundedRect(const QRectF& rect,
                            qreal xRadius,
                            qreal yRadius,
                            const QPen& pen,
                            const QBrush& brush)
{
  Painter(this, pen, brush).drawRoundedRect(normalize(rect), xRadius, yRadius);
}

QRectF
MiniWindow::drawText(const QFont& font,
                     const QString& text,
                     const QRectF& rect,
                     const QColor& color)
{
  Painter painter(this, color);
  painter.setFont(font);
  QRectF boundingRect;
  painter.drawText(normalize(rect), 0, text, &boundingRect);
  return boundingRect;
}

QVariant
MiniWindow::execMenu(const QPoint& location, string_view menuString)
{
  QMenu menu(this);
  const bool returnsNumber = buildMenu(menu, menuString);
  const QAction* choice = menu.exec(mapToGlobal(location));
  if (choice == nullptr) {
    return QStringLiteral("");
  }
  const QString text = choice->text();
  return returnsNumber ? QVariant(text.toDouble()) : QVariant(text);
}

Hotspot*
MiniWindow::findHotspot(string_view hotspotID) const
{
  auto search = hotspots.find(hotspotID);
  if (search == hotspots.end()) {
    return nullptr;
  }
  return &*search->second;
}

const QFont*
MiniWindow::findFont(string_view fontID) const
{
  auto search = fonts.find(fontID);
  if (search == fonts.end()) {
    return nullptr;
  }
  return &search->second;
}

const QPixmap*
MiniWindow::findImage(string_view imageID) const
{
  auto search = images.find(imageID);
  if (search == images.end()) {
    return nullptr;
  }
  return &search->second;
}

void
MiniWindow::invert(const QRect& rectBase, QImage::InvertMode mode)
{
  const QRect rect = normalize(rectBase);
  QImage image = pixmap.copy(rect).toImage();
  image.invertPixels(mode);
  Painter(this, QPainter::CompositionMode_Source).drawImage(rect, image);
}

void
MiniWindow::reset()
{
  if (!flags.testFlag(Flag::KeepHotspots)) {
    clearHotspots();
  }
}

void
MiniWindow::setPosition(const QPoint& loc,
                        Position pos,
                        Flags newFlags) noexcept
{
  location = loc;
  position = pos;
  flags = newFlags;
  applyFlags();
}

void
MiniWindow::setSize(const QSize& size, const QColor& fill) noexcept
{
  background = fill;
  dimensions = size;
}

void
MiniWindow::setZOrder(int64_t order) noexcept
{
  zOrder = order;
}

void
MiniWindow::updatePosition()
{
  const QRect geometry = flags.testFlag(Flag::Absolute)
                           ? QRect(location, dimensions)
                           : geometry::calculate(this, position, dimensions);

  const QSize newSize = geometry.size() * devicePixelRatio();
  if (pixmap.size() != newSize) {
    QPixmap newPixmap(newSize);
    newPixmap.setDevicePixelRatio(devicePixelRatio());
    newPixmap.fill(background);
    QPainter(&newPixmap).drawPixmap(QPointF(), pixmap);
    pixmap.swap(newPixmap);
  }

  setGeometry(geometry);
  updateMask();
}

// Protected overrides

void
MiniWindow::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.setClipRegion(event->region());
  if (position == Position::Tile) [[unlikely]] {
    painter.drawTiledPixmap(rect(), pixmap);
    return;
  }
  int x, y, w, h;
  event->rect().getRect(&x, &y, &w, &h);
  qreal ratio = devicePixelRatio();
  QRectF targetRect(x * ratio, y * ratio, w * ratio, h * ratio);
  painter.drawPixmap(QPointF(x, y), pixmap, targetRect);
}

// Private methods

QRect
MiniWindow::normalize(const QRect& rect) const noexcept
{
  return geometry::normalize(rect, pixmap.size());
}

QRectF
MiniWindow::normalize(const QRectF& rect) const noexcept
{
  return geometry::normalize(rect, pixmap.size());
}

void
MiniWindow::applyFlags()
{
  setAttribute(Qt::WA_TransparentForMouseEvents,
               flags.testAnyFlags(Flag::DrawUnderneath | Flag::IgnoreMouse));

  if (flags.testFlag(Flag::Transparent)) {
    updateMask();
  } else {
    clearMask();
  }
}

void
MiniWindow::updateMask()
{
  if (flags.testFlag(Flag::Transparent)) [[unlikely]] {
    setMask(pixmap.createMaskFromColor(background));
  }
}
