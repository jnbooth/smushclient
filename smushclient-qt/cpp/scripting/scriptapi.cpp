#include "scriptapi.h"
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
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

int ScriptApi::RGBColourToCode(const QColor &color)
{
  if (!color.isValid())
    return -1;
  int r, g, b;
  color.getRgb(&r, &g, &b);
  return b << 16 | g << 8 | r;
}

QColor ScriptApi::RGBCodeToColour(int rgb)
{
  return QColor(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
}

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
    if (!runScript(plugin, *++it))
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

inline bool ScriptApi::handleResult(RunScriptResult result, const Plugin &plugin)
{
  switch (result)
  {
  case RunScriptResult::Ok:
    return true;
  case RunScriptResult::Disabled:
    return false;
  case RunScriptResult::CompileError:
    printError(tr("Compile error: %1").arg(plugin.getError()));
    return false;
  case RunScriptResult::RuntimeError:
    printError(tr("Runtime error: %1").arg(plugin.getError()));
    return false;
  }
}

inline bool ScriptApi::runScript(Plugin &plugin, const QString &script)
{
  return handleResult(plugin.runScript(script), plugin);
}
