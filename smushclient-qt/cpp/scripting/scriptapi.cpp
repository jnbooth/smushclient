#include "scriptapi.h"
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include "miniwindow.h"
#include "worldproperties.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;
using std::unordered_map;

constexpr size_t noSuchPlugin = SIZE_T_MAX;

// Note: because these are called in Lua, use [[unlikely]] for parameter validation failures.

// utils

inline QTextCharFormat colorFormat(const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  if (foreground.isValid())
    format.setForeground(QBrush(foreground));
  if (background.isValid())
    format.setBackground(QBrush(background));
  return format;
}

inline QColor getColorFromVariant(const QVariant &variant)
{
  if (variant.canConvert<QString>())
  {
    QString colorName = variant.toString();
    return colorName.isEmpty() ? QColor::fromRgb(0, 0, 0, 0) : QColor::fromString(colorName);
  }
  bool ok;
  int rgb = variant.toInt(&ok);
  if (!ok || rgb < 0 || rgb > 0xFFFFFF)
    return QColor();
  return QColor(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
}

inline bool isEmptyList(const QVariant &variant)
{
  switch (variant.typeId())
  {
  case QMetaType::QStringList:
    return variant.toStringList().isEmpty();
  case QMetaType::QVariantList:
    return variant.toList().isEmpty();
  default:
    return false;
  }
}

inline ApiCode updateWorld(WorldTab &worldtab)
{
  return worldtab.updateWorld() ? ApiCode::OK : ApiCode::OptionOutOfRange;
}

inline bool beginTell(QTextCursor &cursor, int lastTellPosition)
{
  if (cursor.position() == lastTellPosition)
  {
    cursor.setPosition(lastTellPosition - 1);
    return true;
  }
  return false;
}

inline void endTell(QTextCursor &cursor, bool insideTell)
{
  if (insideTell)
    cursor.setPosition(cursor.position() + 1);
  else
    cursor.insertBlock();
}

// static API

void ScriptApi::SetClipboard(const QString &text)
{
  QGuiApplication::clipboard()->setText(text);
}

// constructor

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      cursor(parent->ui->output->document()),
      lastTellPosition(-1)
{
  applyWorld(parent->world);
}

// public API

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  const bool insideTell = beginTell(cursor, lastTellPosition);
  cursor.insertText(text, colorFormat(foreground, background));
  endTell(cursor, insideTell);
  lastTellPosition = cursor.position();
}

ApiCode ScriptApi::EnableAlias(const QString &label, bool enabled) const
{
  return client()->setAliasEnabled(label, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableAliasGroup(const QString &group, bool enabled) const
{
  return client()->setAliasesEnabled(group, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTimer(const QString &label, bool enabled) const
{
  return client()->setTimerEnabled(label, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnablePlugin(string_view pluginID, bool enabled)
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin)
    return ApiCode::NoSuchPlugin;
  plugins[index].disable();
  client()->setPluginEnabled(index, enabled);
  return ApiCode::OK;
}

ApiCode ScriptApi::EnableTimerGroup(const QString &group, bool enabled) const
{
  return client()->setTimersEnabled(group, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTrigger(const QString &label, bool enabled) const
{
  return client()->setTriggerEnabled(label, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::EnableTriggerGroup(const QString &group, bool enabled) const
{
  return client()->setTriggersEnabled(group, enabled) ? ApiCode::OK : ApiCode::AliasNotFound;
}

QVariant ScriptApi::GetOption(string_view name) const
{
  const char *prop = WorldProperties::canonicalName(name);
  if (prop == nullptr)
    return QVariant();

  return tab()->world.property(prop);
}

optional<string_view> ScriptApi::GetVariable(size_t index, string_view key) const
{
  size_t size;
  const char *variable = client()->getVariable(index, key.data(), key.size(), &size);
  if (variable == nullptr)
    return nullopt;
  return string_view(variable, size);
}

optional<string_view> ScriptApi::GetVariable(string_view pluginID, string_view key) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin)
    return nullopt;
  return GetVariable(index, key);
}

const QString &ScriptApi::GetPluginId(size_t index) const
{
  return plugins.at(index).id();
}

QVariant ScriptApi::GetPluginInfo(string_view pluginID, uint8_t infoType) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]]
    return QVariant();
  switch (infoType)
  {
  case 16:
    return QVariant(!plugins[index].disabled());
  default:
    return client()->pluginInfo(index, infoType);
  }
}

void ScriptApi::Hyperlink(
    const QString &action,
    const QString &text,
    const QString &hint,
    const QColor &foreground,
    const QColor &background,
    bool url,
    bool noUnderline)
{
  QTextCharFormat format;
  if (url)
  {
    QString link = action;
    link.prepend(QChar(17));
    format.setAnchorHref(link);
  }
  else
    format.setAnchorHref(action);
  format.setToolTip(hint.isEmpty() ? action : hint);
  if (foreground.isValid())
    format.setForeground(foreground);
  if (background.isValid())
    format.setBackground(background);
  if (!noUnderline)
    format.setAnchor(true);
  cursor.insertText(text, format);
}

ApiCode ScriptApi::IsAlias(const QString &label) const
{
  return client()->isAlias(label) ? ApiCode::OK : ApiCode::AliasNotFound;
}

ApiCode ScriptApi::IsTimer(const QString &label) const
{
  return client()->isTimer(label) ? ApiCode::OK : ApiCode::TimerNotFound;
}

ApiCode ScriptApi::IsTrigger(const QString &label) const
{
  return client()->isTrigger(label) ? ApiCode::OK : ApiCode::TriggerNotFound;
}

ApiCode ScriptApi::Send(const QByteArrayView &view)
{
  echo(QString::fromUtf8(view));
  return SendNoEcho(view);
}

ApiCode ScriptApi::SendNoEcho(const QByteArrayView &view) const
{
  if (view.isEmpty()) [[unlikely]]
    return ApiCode::OK;

  QTcpSocket &socket = *tab()->socket;

  if (!socket.isOpen()) [[unlikely]]
    return ApiCode::WorldClosed;

  if (view.back() == '\n')
    socket.write(view.constData(), view.size());

  else
  {
    QByteArray bytes;
    bytes.reserve(view.size() + 1);
    bytes.append(view);
    bytes.append('\n');
    socket.write(bytes);
  }
  return ApiCode::OK;
}

ApiCode ScriptApi::SetOption(string_view name, const QVariant &variant) const
{
  WorldTab &worldtab = *tab();
  World &world = worldtab.world;
  const char *prop = WorldProperties::canonicalName(name);
  if (prop == nullptr) [[unlikely]]
    return ApiCode::UnknownOption;
  QVariant property = world.property(prop);
  if (world.setProperty(prop, variant))
    return updateWorld(worldtab);

  switch (property.typeId())
  {
  case QMetaType::QColor:
    if (QColor color = getColorFromVariant(variant); color.isValid() && world.setProperty(prop, color))
      return updateWorld(worldtab);
  case QMetaType::QVariantHash:
    if (isEmptyList(variant) && world.setProperty(prop, QVariantHash()))
      return updateWorld(worldtab);
  case QMetaType::QVariantMap:
    if (isEmptyList(variant) && world.setProperty(prop, QVariantMap()))
      return updateWorld(worldtab);
  }

  return ApiCode::OptionOutOfRange;
}

bool ScriptApi::SetVariable(size_t index, string_view key, string_view value) const
{
  return client()->setVariable(index, key.data(), key.size(), value.data(), value.size());
}

void ScriptApi::Tell(const QString &text)
{
  const bool insideTell = beginTell(cursor, lastTellPosition);
  cursor.insertText(text);
  endTell(cursor, insideTell);
  lastTellPosition = cursor.position();
}

ApiCode ScriptApi::TextRectangle(
    const QMargins &margins,
    int borderOffset,
    const QColor &borderColor,
    int borderWidth,
    const QBrush &outsideFill) const
{
  Ui::WorldTab *ui = tab()->ui;
  ui->area->setContentsMargins(margins);
  QPalette areaPalette = ui->area->palette();
  areaPalette.setBrush(QPalette::ColorRole::Base, outsideFill);
  ui->area->setPalette(areaPalette);
  ui->outputBorder->setContentsMargins(borderWidth, borderWidth, borderWidth, borderWidth);
  QPalette borderPalette = ui->outputBorder->palette();
  borderPalette.setBrush(QPalette::ColorRole::Base, borderColor);
  ui->outputBorder->setPalette(borderPalette);
  ui->background->setContentsMargins(borderOffset, borderOffset, borderOffset, borderOffset);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowAddHotspot(
    string_view pluginID,
    string_view windowName,
    string_view hotspotID,
    const QRect &geometry,
    Hotspot::Callbacks &&callbacks,
    const QString &tooltip,
    Qt::CursorShape cursor,
    bool trackHover) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  const size_t pluginIndex = findPluginIndex(pluginID);
  if (pluginIndex == noSuchPlugin) [[unlikely]]
    return ApiCode::NoSuchPlugin;
  const Plugin *plugin = &plugins[findPluginIndex(pluginID)];
  Hotspot *hotspot = window->addHotspot(hotspotID, plugin, std::move(callbacks));
  if (hotspot == nullptr) [[unlikely]]
    return ApiCode::HotspotPluginChanged;
  hotspot->setGeometry(geometry);
  hotspot->setToolTip(tooltip);
  hotspot->setCursor(cursor);
  hotspot->setMouseTracking(trackHover);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowCreate(
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

  string windowName = (string)name;
  MiniWindow *window = windows[windowName];
  if (window == nullptr)
    window = windows[windowName] =
        new MiniWindow(tab()->ui->area, location, size, position, flags, fill);
  else
  {
    window->setPosition(location, position, flags);
    window->setSize(size, fill);
    window->reset();
  }
  window->updatePosition();
  stackWindow(name, window);
  window->show();
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowDeleteHotspot(string_view windowName, string_view hotspotID) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  if (!window->deleteHotspot(hotspotID)) [[unlikely]]
    return ApiCode::HotspotNotInstalled;
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowEllipse(
    string_view windowName,
    const QRectF &rect,
    const QPen &pen,
    const QBrush &brush) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->drawEllipse(rect, pen, brush);
  return ApiCode::OK;
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
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  QFont font(fontName, pointSize, bold ? QFont::Bold : QFont::Normal, italic);
  font.setStyleHint(hint);
  font.setPointSizeF(pointSize);
  if (underline)
    font.setUnderline(true);
  if (strikeout)
    font.setStrikeOut(true);
  window->loadFont(fontID, font);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowFrame(
    string_view windowName,
    const QRectF &rect,
    const QColor &color1,
    const QColor &color2) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr || source == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->loadImage(imageID, source->getPixmap());
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowLine(
    string_view windowName,
    const QLineF &line,
    const QPen &pen) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
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

ApiCode ScriptApi::WindowMoveHotspot(
    string_view windowName,
    string_view hotspotID,
    const QRect &geometry) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  Hotspot *hotspot = window->findHotspot(hotspotID);
  if (hotspot == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setPosition(location, position, flags);
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
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setSize(size, fill);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowSetZOrder(string_view windowName, int order) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->setZOrder(order);
  stackWindow(windowName, window);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowShow(string_view windowName, bool show) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
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
  if (window == nullptr) [[unlikely]]
    return -1;
  const QFont *font = window->getFont(fontID);
  if (font == nullptr) [[unlikely]]
    return -2;
  return window->drawText(*font, text, rect, color).width();
}

int ScriptApi::WindowTextWidth(
    string_view windowName,
    string_view fontID,
    const QString &text) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return -1;
  const QFont *font = window->getFont(fontID);
  if (font == nullptr) [[unlikely]]
    return -2;
  QFontMetrics fm(*font);
  return fm.horizontalAdvance(text);
}

ApiCode ScriptApi::WindowUnloadFont(string_view windowName, string_view fontID) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->unloadFont(fontID);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowUnloadImage(string_view windowName, string_view windowID) const
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr) [[unlikely]]
    return ApiCode::NoSuchWindow;
  window->unloadImage(windowID);
  return ApiCode::OK;
}

// public methods

void ScriptApi::applyWorld(const World &world)
{
  QTextCharFormat noteFormat;
  noteFormat.setForeground(QBrush(world.getCustomColor()));
  cursor.setCharFormat(noteFormat);
  echoFormat.setForeground(QBrush(world.getEchoTextColour()));
  echoFormat.setBackground(QBrush(world.getEchoBackgroundColour()));
  errorFormat.setForeground(QBrush(world.getErrorColour()));
}

SmushClient *ScriptApi::client() const
{
  WorldTab *worldtab = tab();
  if (worldtab == nullptr) [[unlikely]]
    return nullptr;
  return &worldtab->client;
}

void ScriptApi::echo(const QString &text)
{
  cursor.insertText(text, echoFormat);
  cursor.insertBlock();
}

void ScriptApi::finishNote()
{
  lastTellPosition = -1;
}

const Plugin *ScriptApi::getPlugin(string_view pluginID) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]]
    return nullptr;
  return &plugins[index];
}

void ScriptApi::initializeScripts(const QStringList &scripts)
{
  if (!windows.empty())
  {
    for (const auto &entry : windows)
      delete entry.second;
    windows.clear();
  }
  size_t size = scripts.size();
  plugins.clear();
  plugins.reserve(size);
  pluginIndices.clear();
  pluginIndices.reserve(size);
  QString error;
  for (auto start = scripts.cbegin(), it = start, end = scripts.cend(); it != end; ++it)
  {
    PluginMetadata metadata{
        .id = *it,
        .index = (size_t)(it - start),
        .name = *++it,
    };
    pluginIndices[metadata.id.toStdString()] = metadata.index;
    Plugin &plugin = plugins.emplace_back(this, std::move(metadata));
    if (!plugin.runScript(*++it))
      plugin.disable();
  }
}

void ScriptApi::printError(const QString &error)
{
  cursor.insertText(error, errorFormat);
  cursor.insertBlock();
}

// private methods

size_t ScriptApi::findPluginIndex(string_view pluginID) const
{
  auto search = pluginIndices.find((string)pluginID);
  if (search == pluginIndices.end()) [[unlikely]]
    return noSuchPlugin;
  return search->second;
}

MiniWindow *ScriptApi::findWindow(string_view windowName) const
{
  auto search = windows.find((string)windowName);
  if (search == windows.end()) [[unlikely]]
    return nullptr;
  return search->second;
}

struct WindowCompare
{
  int zOrder;
  string_view name;
  std::strong_ordering operator<=>(const WindowCompare &) const = default;
};

void ScriptApi::stackWindow(string_view windowName, MiniWindow *window) const
{
  const bool drawsUnderneath = window->drawsUnderneath();
  const WindowCompare compare{window->getZOrder(), windowName};
  MiniWindow *neighbor = nullptr;
  WindowCompare neighborCompare;

  for (const auto &entry : windows)
  {
    if (entry.second == window || entry.second->drawsUnderneath() != drawsUnderneath)
      continue;
    WindowCompare entryCompare{entry.second->getZOrder(), (string_view)entry.first};
    if (entryCompare > compare && (!neighbor || entryCompare < neighborCompare))
    {
      neighbor = entry.second;
      neighborCompare = entryCompare;
    }
  }

  if (neighbor != nullptr)
    window->stackUnder(neighbor);
  else if (drawsUnderneath)
    window->stackUnder(tab()->ui->outputBorder);
}
