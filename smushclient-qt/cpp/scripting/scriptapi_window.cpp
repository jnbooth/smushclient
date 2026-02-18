#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "miniwindow/miniwindow.h"
#include "scriptapi.h"
#include "sqlite3.h"
#include <QtCore/QFile>
#include <QtGui/QFontDatabase>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QStatusBar>
#include <memory>

using std::string;
using std::string_view;
using std::unique_ptr;

// Private utils

#define TRY_WINDOW(windowName)                                                 \
  findWindow(windowName);                                                      \
  if (window == nullptr) [[unlikely]] {                                        \
    return ApiCode::NoSuchWindow;                                              \
  }

#define TRY_IMAGE(imageID)                                                     \
  window->findImage(imageID);                                                  \
  if (image == nullptr) [[unlikely]] {                                         \
    return ApiCode::ImageNotInstalled;                                         \
  }

#define TRY_HOTSPOT(hotspotID)                                                 \
  window->findHotspot(hotspotID);                                              \
  if (hotspot == nullptr) [[unlikely]] {                                       \
    return ApiCode::HotspotNotInstalled;                                       \
  }

#define CHECK_NONNULL(ptr)                                                     \
  if ((ptr) == nullptr) [[unlikely]] {                                         \
    return QVariant();                                                         \
  }

namespace {
bool
setExactFontFamily(QFont& font, const QString& family)
{
  font.setFamily(family);
  return QFontInfo(font).family() == family;
}

void
assignFontFamily(QFont& font, const QString& family)
{
  if (setExactFontFamily(font, family)) {
    return;
  }
  if (family == QStringLiteral("FixedSys") &&
      setExactFontFamily(font, QStringLiteral("Fixedsys"))) {
    return;
  }
  font.setFamily(
    QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont).family());
}
} // namespace

// Public methods

ApiCode
ScriptApi::WindowAddHotspot(size_t index,
                            string_view windowName,
                            string_view hotspotID,
                            const QRect& geometry,
                            Hotspot::Callbacks&& callbacks,
                            const QString& tooltip,
                            Qt::CursorShape cursorShape,
                            Hotspot::Flags flags) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot =
    window->addHotspot(hotspotID, tab, plugins[index], std::move(callbacks));
  if (hotspot == nullptr) [[unlikely]] {
    return ApiCode::HotspotPluginChanged;
  }
  hotspot->setGeometry(geometry);
  hotspot->setToolTip(tooltip);
  hotspot->setCursor(cursorShape);
  hotspot->setMouseTracking(flags.testFlag(Hotspot::Flag::ReportAllMouseovers));
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowBlendImage(string_view windowName,
                            string_view imageID,
                            const QRectF& rect,
                            BlendMode mode,
                            qreal opacity,
                            const QRectF& sourceRect) const
{
  if (opacity < 0 || opacity > 1) [[unlikely]] {
    return ApiCode::BadParameter;
  }
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* image = TRY_IMAGE(imageID);
  window->blendImage(mode, *image, rect, opacity, sourceRect);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowButton(string_view windowName,
                        const QRect& rect,
                        MiniWindow::ButtonFrame frame,
                        MiniWindow::ButtonFlags flags) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawButton(rect, frame, flags);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowCreate(size_t index,
                        string_view name,
                        const QPoint& location,
                        const QSize& size,
                        MiniWindow::Position position,
                        MiniWindow::Flags flags,
                        const QColor& fill)
{
  if (name.empty()) [[unlikely]] {
    return ApiCode::NoNameSpecified;
  }
  if (!size.isValid()) [[unlikely]] {
    return ApiCode::BadParameter;
  }

  const string windowName(name);
  std::unique_ptr<MiniWindow>& window = windows[windowName];
  if (window == nullptr) {
    window = std::make_unique<MiniWindow>(
      location, size, position, flags, fill, plugins[index].id(), tab.ui->area);
  } else {
    window->setPosition(location, position, flags);
    window->setSize(size, fill);
    window->reset();
  }
  window->updatePosition();
  stackWindow(name, *window);
  window->hide();
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDeleteHotspot(string_view windowName,
                               string_view hotspotID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  if (!window->deleteHotspot(hotspotID)) [[unlikely]] {
    return ApiCode::HotspotNotInstalled;
  }
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDrawImage(string_view windowName,
                           string_view imageID,
                           const QRectF& rect,
                           MiniWindow::DrawImageMode mode,
                           const QRectF& sourceRect) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* image = TRY_IMAGE(imageID);
  window->drawImage(*image, rect, sourceRect, mode);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowDrawImageAlpha(string_view windowName,
                                string_view imageID,
                                const QRectF& rect,
                                qreal opacity,
                                const QPointF& origin) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const QPixmap* image = TRY_IMAGE(imageID);
  window->drawImage(*image,
                    rect,
                    QRectF(origin, image->rect().bottomRight().toPointF()),
                    MiniWindow::DrawImageMode::Copy,
                    opacity);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowEllipse(string_view windowName,
                         const QRectF& rect,
                         const QPen& pen,
                         const QBrush& brush) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawEllipse(rect, pen, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowFilter(string_view windowName,
                        const ImageFilter& filter,
                        const QRect& rect) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->applyFilter(filter, rect);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowFont(string_view windowName,
                      string_view fontID,
                      const QString& family,
                      qreal pointSize,
                      bool bold,
                      bool italic,
                      bool underline,
                      bool strikeout,
                      QFont::StyleHint hint) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  QFont font;
  font.setStyleHint(hint);
  font.setPointSizeF(pointSize);
  if (bold) {
    font.setBold(true);
  }
  if (italic) {
    font.setItalic(true);
  }
  if (underline) {
    font.setUnderline(true);
  }
  if (strikeout) {
    font.setStrikeOut(true);
  }
  assignFontFamily(font, family);
  window->loadFont(fontID, font);
  return ApiCode::OK;
}

QVariant
ScriptApi::WindowFontInfo(string_view windowName,
                          string_view fontID,
                          int64_t infoType) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  const QFont* font = window->findFont(fontID);
  CHECK_NONNULL(font);
  return FontInfo(*font, infoType);
}

QVariant
ScriptApi::WindowHotspotInfo(string_view windowName,
                             string_view hotspotID,
                             int64_t infoType) const
{

  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  const Hotspot* hotspot = window->findHotspot(hotspotID);
  CHECK_NONNULL(hotspot);
  return hotspot->info(infoType);
}

ApiCode
ScriptApi::WindowFrame(string_view windowName,
                       const QRectF& rect,
                       const QColor& color1,
                       const QColor& color2) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawFrame(rect, color1, color2);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowGradient(string_view windowName,
                          const QRectF& rect,
                          const QColor& color1,
                          const QColor& color2,
                          Qt::Orientation direction) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  const qreal horizontalStop = direction == Qt::Orientation::Horizontal ? 1 : 0;
  QLinearGradient gradient(0, 0, horizontalStop, 1 - horizontalStop);
  gradient.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
  gradient.setColorAt(0, color1);
  gradient.setColorAt(1, color2);
  window->drawGradient(rect, gradient);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowImageFromWindow(string_view windowName,
                                 string_view imageID,
                                 string_view sourceWindow) const
{
  MiniWindow* window = findWindow(windowName);
  MiniWindow* source = findWindow(sourceWindow);
  if ((window == nullptr) || (source == nullptr)) [[unlikely]] {
    return ApiCode::NoSuchWindow;
  }
  window->loadImage(imageID, source->getPixmap());
  return ApiCode::OK;
}

QVariant
ScriptApi::WindowInfo(string_view windowName, int64_t infoType) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  return window->info(infoType);
}

ApiCode
ScriptApi::WindowInvert(string_view windowName, const QRect& rect) const
{

  MiniWindow* window = TRY_WINDOW(windowName);
  window->invert(rect);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowLine(string_view windowName,
                      const QLineF& line,
                      const QPen& pen) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawLine(line, pen);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowLoadImage(string_view windowName,
                           string_view imageID,
                           const QString& filename) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  QPixmap image(filename);
  if (!image.isNull()) [[likely]] {
    window->loadImage(imageID, std::move(image));
    return ApiCode::OK;
  }
  if (QFile::exists(filename)) {
    return ApiCode::UnableToLoadImage;
  }
  return ApiCode::FileNotFound;
}

QVariant
ScriptApi::WindowMenu(string_view windowName,
                      const QPoint& location,
                      string_view menuString) const
{
  MiniWindow* window = findWindow(windowName);
  CHECK_NONNULL(window);
  return window->execMenu(location, menuString);
}

ApiCode
ScriptApi::WindowMoveHotspot(string_view windowName,
                             string_view hotspotID,
                             const QRect& geometry) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot = TRY_HOTSPOT(hotspotID);
  hotspot->setGeometry(geometry);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowPolygon(string_view windowName,
                         const QPolygonF& polygon,
                         const QPen& pen,
                         const QBrush& brush,
                         bool close,
                         Qt::FillRule fillRule) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  if (brush.style() == Qt::BrushStyle::NoBrush && !close) {
    window->drawPolyline(polygon, pen);
  } else {
    window->drawPolygon(polygon, pen, brush, fillRule);
  }
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowPosition(string_view windowName,
                          const QPoint& location,
                          MiniWindow::Position position,
                          MiniWindow::Flags flags) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setPosition(location, position, flags);
  window->updatePosition();
  stackWindow(windowName, *window);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowRect(string_view windowName,
                      const QRectF& rect,
                      const QPen& pen,
                      const QBrush& brush) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawRect(rect, pen, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowRoundedRect(string_view windowName,
                             const QRectF& rect,
                             qreal xRadius,
                             qreal yRadius,
                             const QPen& pen,
                             const QBrush& brush) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->drawRoundedRect(rect, xRadius, yRadius, pen, brush);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowResize(string_view windowName,
                        const QSize& size,
                        const QColor& fill) const
{
  if (windowName.empty()) {
    return ApiCode::NoNameSpecified;
  }
  if (!size.isValid()) {
    return ApiCode::BadParameter;
  }
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setSize(size, fill);
  window->updatePosition();
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowSetZOrder(string_view windowName, int64_t order) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setZOrder(order);
  stackWindow(windowName, *window);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowShow(string_view windowName, bool show) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->setVisible(show);
  return ApiCode::OK;
}

qreal
ScriptApi::WindowText(string_view windowName,
                      string_view fontID,
                      string_view text,
                      const QRectF& rect,
                      const QColor& color,
                      bool unicode) const
{
  MiniWindow* window = findWindow(windowName);
  if (window == nullptr) [[unlikely]] {
    return -1;
  }
  const QFont* font = window->findFont(fontID);
  if (font == nullptr) [[unlikely]] {
    return -2;
  }
  const QString qtext =
    unicode ? QString::fromUtf8(text) : QString::fromLatin1(text);
  return window->drawText(*font, qtext, rect, color).width();
}

int
ScriptApi::WindowTextWidth(string_view windowName,
                           string_view fontID,
                           string_view text,
                           bool unicode) const
{
  MiniWindow* window = findWindow(windowName);
  if (window == nullptr) [[unlikely]] {
    return -1;
  }
  const QFont* font = window->findFont(fontID);
  if (font == nullptr) [[unlikely]] {
    return -2;
  }
  QFontMetrics fm(*font);
  const QString qtext =
    unicode ? QString::fromUtf8(text) : QString::fromLatin1(text);
  return fm.horizontalAdvance(qtext);
}

ApiCode
ScriptApi::WindowUnloadFont(string_view windowName, string_view fontID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->unloadFont(fontID);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowUnloadImage(string_view windowName, string_view windowID) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  window->unloadImage(windowID);
  return ApiCode::OK;
}

ApiCode
ScriptApi::WindowUpdateHotspot(size_t index,
                               string_view windowName,
                               string_view hotspotID,
                               Hotspot::CallbacksPartial&& callbacks) const
{
  MiniWindow* window = TRY_WINDOW(windowName);
  Hotspot* hotspot = TRY_HOTSPOT(hotspotID);
  if (!hotspot->belongsToPlugin(plugins[index])) {
    return ApiCode::HotspotPluginChanged;
  }
  hotspot->setCallbacks(std::move(callbacks));
  return ApiCode::OK;
}
