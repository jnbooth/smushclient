#include "scriptapi.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include "miniwindow.h"
#include "worldproperties.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

using std::string;
using std::string_view;
using std::unordered_map;

constexpr size_t noSuchPlugin = SIZE_T_MAX;

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

QVariant ScriptApi::GetPluginInfo(string_view pluginID, uint8_t infoType) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin)
    return QVariant();
  switch (infoType)
  {
  case 16:
    return QVariant(!plugins[index].disabled());
  case 12:
    return QVariant((qlonglong)plugins[index].variables()->size());
  default:
    return client()->pluginInfo(index, infoType);
  }
}

void ScriptApi::Hyperlink(const QString &action, const QString &text, const QString &hint, const QColor &foreground, const QColor &background, bool url, bool noUnderline)
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
  if (view.isEmpty())
    return ApiCode::OK;

  QTcpSocket &socket = *tab()->socket;

  if (!socket.isOpen())
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
  if (prop == nullptr)
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

void ScriptApi::Tell(const QString &text)
{
  const bool insideTell = beginTell(cursor, lastTellPosition);
  cursor.insertText(text);
  endTell(cursor, insideTell);
  lastTellPosition = cursor.position();
}

ApiCode ScriptApi::WindowAddHotspot(
    string_view pluginID,
    string_view windowName,
    string_view hotspotID,
    const QRect &geometry,
    Hotspot::Callbacks &&callbacks,
    const QString &tooltip,
    Qt::CursorShape cursor,
    bool trackHover)
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr)
    return ApiCode::NoSuchWindow;
  const size_t pluginIndex = findPluginIndex(pluginID);
  if (pluginIndex == noSuchPlugin)
    return ApiCode::NoSuchPlugin;
  const Plugin *plugin = &plugins[findPluginIndex(pluginID)];
  Hotspot *hotspot = window->addHotspot(hotspotID, plugin, std::move(callbacks));
  if (hotspot == nullptr)
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
  if (name.empty())
    return ApiCode::NoNameSpecified;
  if (!size.isValid())
    return ApiCode::BadParameter;

  string windowName = (string)name;
  MiniWindow *window = windows[windowName];
  if (window == nullptr)
    window = windows[windowName] = new MiniWindow(tab(), location, size, position, flags, fill);
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
  if (window == nullptr)
    return ApiCode::NoSuchWindow;
  if (!window->deleteHotspot(hotspotID))
    return ApiCode::HotspotNotInstalled;
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
  if (window == nullptr)
    return ApiCode::NoSuchWindow;
  window->setPosition(location, position, flags);
  stackWindow(windowName, window);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowResize(string_view windowName, const QSize &size, const QColor &fill) const
{
  if (windowName.empty())
    return ApiCode::NoNameSpecified;
  if (!size.isValid())
    return ApiCode::BadParameter;
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr)
    return ApiCode::NoSuchWindow;
  window->setSize(size, fill);
  return ApiCode::OK;
}

ApiCode ScriptApi::WindowSetZOrder(string_view windowName, int order)
{
  MiniWindow *window = findWindow(windowName);
  if (window == nullptr)
    return ApiCode::NoSuchWindow;
  window->setZOrder(order);
  stackWindow(windowName, window);
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
  if (worldtab == nullptr)
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
  if (index == noSuchPlugin)
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
        .name = *++it,
    };
    pluginIndices[metadata.id.toStdString()] = it - start;
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
  if (search == pluginIndices.end())
    return noSuchPlugin;
  return search->second;
}

MiniWindow *ScriptApi::findWindow(string_view windowName) const
{
  auto search = windows.find((string)windowName);
  if (search == windows.end())
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
  MiniWindow *neighbor;
  WindowCompare neighborCompare;

  for (const auto &entry : windows)
  {
    if (entry.second->drawsUnderneath() != drawsUnderneath)
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
    window->stackUnder(tab()->ui->output);
}
