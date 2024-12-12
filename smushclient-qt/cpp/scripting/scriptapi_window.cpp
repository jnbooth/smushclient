#include "scriptapi.h"
#include <QtCore/QFile>
#include <QtGui/QFontDatabase>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QStatusBar>
#include "sqlite3.h"
#include "miniwindow.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;

// Public methods

ApiCode ScriptApi::WindowAddHotspot(
    size_t index,
    string_view windowName,
    string_view hotspotID,
    const QRect &geometry,
    Hotspot::Callbacks &&callbacks,
    const QString &tooltip,
    Qt::CursorShape cursorShape,
    Hotspot::Flags flags) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  const Plugin *plugin = &plugins[index];
  Hotspot *hotspot = window->addHotspot(hotspotID, tab(), plugin, std::move(callbacks));
  if (!hotspot) [[unlikely]]
    return ApiCode::HotspotPluginChanged;
  hotspot->setGeometry(geometry);
  hotspot->setToolTip(tooltip);
  hotspot->setCursor(cursorShape);
  hotspot->setMouseTracking(flags.testFlag(Hotspot::Flag::ReportAllMouseovers));
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowButton(
    std::string_view windowName,
    const QRect &rect,
    MiniWindow::ButtonFrame frame,
    MiniWindow::ButtonFlags flags) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawButton(rect, frame, flags);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowCreate(
    size_t index,
    string_view name,
    const QPoint &location,
    const QSize &size,
    MiniWindow::Position position,
    MiniWindow::Flags flags,
    const QColor &fill)
{
  if (name.empty()) [[unlikely]]
    return ApiCode::NoNameSpecified;
  if (!size.isValid()) [[unlikely]]
    return ApiCode::BadParameter;

  const string windowName = (string)name;
  MiniWindow *window = windows[windowName];
  if (!window)
    window = windows[windowName] =
        new MiniWindow(tab()->ui->area, location, size, position, flags, fill, plugins[index].id());
  else
  {
    window->setPosition(location, position, flags);
    window->setSize(size, fill);
    window->reset();
  }
  window->updatePosition();
  stackWindow(name, window);
  window->hide();
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowDeleteHotspot(string_view windowName, string_view hotspotID) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  if (!window->deleteHotspot(hotspotID)) [[unlikely]]
    return ApiCode::HotspotNotInstalled;
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowDrawImage(
    std::string_view windowName,
    std::string_view imageID,
    const QRectF &rect,
    MiniWindow::DrawImageMode mode,
    const QRectF &sourceRect) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  const QPixmap *image = window->findImage(imageID);
  if (!image) [[unlikely]]
    return ApiCode::ImageNotInstalled;
  window->drawImage(*image, rect, sourceRect, mode);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowDrawImageAlpha(
    std::string_view windowName,
    std::string_view imageID,
    const QRectF &rect,
    qreal opacity,
    const QPointF origin) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  const QPixmap *image = window->findImage(imageID);
  if (!image) [[unlikely]]
    return ApiCode::ImageNotInstalled;
  window->drawImage(
      *image,
      rect,
      QRectF(origin, image->rect().bottomRight().toPointF()),
      MiniWindow::DrawImageMode::Copy,
      opacity);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowEllipse(
    string_view windowName,
    const QRectF &rect,
    const QPen &pen,
    const QBrush &brush) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawEllipse(rect, pen, brush);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowFilter(
    string_view windowName,
    const ImageFilter &filter,
    const QRect &rect) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->applyFilter(filter, rect);
  return ApiCode::OK;
}

void assignFontFamily(QFont &font, const QString &fontName)
{
  font.setFamily(fontName);
  if (font.exactMatch())
    return;
  if (fontName == QStringLiteral("FixedSys"))
  {
    font.setFamily(QStringLiteral("Fixedsys"));
    if (font.exactMatch())
      return;
  }
  font.setFamily(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont).family());
}

ApiCode ScriptApi::WindowFont(
    string_view windowName,
    string_view fontID,
    const QString &fontName,
    qreal pointSize,
    bool bold,
    bool italic,
    bool underline,
    bool strikeout,
    QFont::StyleHint hint) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  QFont font;
  assignFontFamily(font, fontName);
  font.setStyleHint(hint);
  font.setPointSizeF(pointSize);
  if (bold)
    font.setBold(true);
  if (italic)
    font.setItalic(true);
  if (underline)
    font.setUnderline(true);
  if (strikeout)
    font.setStrikeOut(true);
  window->loadFont(fontID, font);
  return ApiCode::OK;
}

QVariant ScriptApi::WindowFontInfo(
    string_view windowName,
    string_view fontID,
    int infoType) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return QVariant();
  const QFont *font = window->findFont(fontID);
  if (!font) [[unlikely]]
    return QVariant();
  return FontInfo(*font, infoType);
}

QVariant ScriptApi::WindowHotspotInfo(
    string_view windowName,
    string_view hotspotID,
    int infoType) const
{

  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return QVariant();
  const Hotspot *hotspot = window->findHotspot(hotspotID);
  if (!hotspot) [[unlikely]]
    return QVariant();
  return hotspot->info(infoType);
}

ApiCode ScriptApi::WindowFrame(
    string_view windowName,
    const QRectF &rect,
    const QColor &color1,
    const QColor &color2) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawFrame(rect, color1, color2);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowGradient(
    string_view windowName,
    const QRectF &rect,
    const QColor &color1,
    const QColor &color2,
    Qt::Orientation direction) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  const bool isHorizontal = direction == Qt::Orientation::Horizontal;
  QLinearGradient gradient(0, 0, isHorizontal, !isHorizontal);
  gradient.setCoordinateMode(QGradient::CoordinateMode::ObjectMode);
  gradient.setColorAt(0, color1);
  gradient.setColorAt(1, color2);
  window->drawGradient(rect, gradient);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowImageFromWindow(
    string_view windowName,
    string_view imageID,
    string_view sourceWindow) const
{
  MiniWindow *window = findWindow(windowName);
  MiniWindow *source = findWindow(sourceWindow);
  if (!window || !source) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->loadImage(imageID, source->getPixmap());
  return ApiCode::OK;
}

QVariant ScriptApi::WindowInfo(std::string_view windowName, int infoType) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return QVariant();
  return window->info(infoType);
}

ApiCode ScriptApi::WindowInvert(string_view windowName, const QRect &rect) const
{

  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->invert(rect);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowLine(
    string_view windowName,
    const QLineF &line,
    const QPen &pen) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawLine(line, pen);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowLoadImage(
    string_view windowName,
    string_view imageID,
    const QString &filename) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  const QPixmap image(filename);
  if (!image.isNull()) [[likely]]
  {
    window->loadImage(imageID, std::move(image));
    return ApiCode::OK;
  }
  if (QFile::exists(filename))
    return ApiCode::UnableToLoadImage;
  return ApiCode::FileNotFound;
}

QVariant ScriptApi::WindowMenu(
    std::string_view windowName,
    const QPoint &location,
    std::string_view menuString) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return QVariant();
  return window->execMenu(location, menuString);
}

ApiCode ScriptApi::WindowMoveHotspot(
    string_view windowName,
    string_view hotspotID,
    const QRect &geometry) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  Hotspot *hotspot = window->findHotspot(hotspotID);
  if (!hotspot) [[unlikely]]
    return ApiCode::HotspotNotInstalled;
  hotspot->setGeometry(geometry);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowPolygon(
    string_view windowName,
    const QPolygonF &polygon,
    const QPen &pen,
    const QBrush &brush,
    bool close,
    Qt::FillRule fillRule) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  if (brush.style() == Qt::BrushStyle::NoBrush && !close)
    window->drawPolyline(polygon, pen);
  else
    window->drawPolygon(polygon, pen, brush, fillRule);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowPosition(
    string_view windowName,
    const QPoint &location,
    MiniWindow::Position position,
    MiniWindow::Flags flags)
    const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setPosition(location, position, flags);
  window->updatePosition();
  stackWindow(windowName, window);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowRect(
    string_view windowName,
    const QRectF &rect,
    const QPen &pen,
    const QBrush &brush) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawRect(rect, pen, brush);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowRoundedRect(
    string_view windowName,
    const QRectF &rect,
    qreal xRadius,
    qreal yRadius,
    const QPen &pen,
    const QBrush &brush) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawRoundedRect(rect, xRadius, yRadius, pen, brush);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowResize(string_view windowName, const QSize &size, const QColor &fill) const
{
  if (windowName.empty())
    return ApiCode::NoNameSpecified;
  if (!size.isValid())
    return ApiCode::BadParameter;
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setSize(size, fill);
  window->updatePosition();
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowSetZOrder(string_view windowName, int order) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setZOrder(order);
  stackWindow(windowName, window);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowShow(string_view windowName, bool show) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setVisible(show);
  return ApiCode::OK;
}

qreal ScriptApi::WindowText(
    string_view windowName,
    string_view fontID,
    const QString &text,
    const QRectF &rect,
    const QColor &color) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return -1;
  const QFont *font = window->findFont(fontID);
  if (!font) [[unlikely]]
    return -2;
  return window->drawText(*font, text, rect, color).width();
}

int ScriptApi::WindowTextWidth(
    string_view windowName,
    string_view fontID,
    const QString &text) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return -1;
  const QFont *font = window->findFont(fontID);
  if (!font) [[unlikely]]
    return -2;
  QFontMetrics fm(*font);
  return fm.horizontalAdvance(text);
}

ApiCode ScriptApi::WindowUnloadFont(string_view windowName, string_view fontID) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->unloadFont(fontID);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowUnloadImage(string_view windowName, string_view windowID) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->unloadImage(windowID);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowUpdateHotspot(
    size_t index,
    string_view windowName,
    string_view hotspotID,
    Hotspot::CallbacksPartial &&callbacks) const
{
  MiniWindow *window = findWindow(windowName);
  if (!window) [[unlikely]]
    return ApiCode::NoSuchWindow;
  Hotspot *hotspot = window->findHotspot(hotspotID);
  if (!hotspot) [[unlikely]]
    return ApiCode::HotspotNotInstalled;
  if (!hotspot->belongsToPlugin(&plugins[index]))
    return ApiCode::HotspotPluginChanged;
  hotspot->setCallbacks(std::move(callbacks));
  return ApiCode::OK;
}
