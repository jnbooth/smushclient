#include "scriptapi.h"
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QStatusBar>
#include "sqlite3.h"
#include "miniwindow.h"
#include "worldproperties.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;

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

void ScriptApi::SetClipboard(const QString &text)
{
  QGuiApplication::clipboard()->setText(text);
}

int ScriptApi::BroadcastPlugin(size_t index, int message, string_view text) const
{
  const Plugin &callingPlugin = plugins[index];
  const string pluginID = callingPlugin.id().toStdString();
  const string pluginName = callingPlugin.name().toStdString();
  int calledPlugins = 0;
  OnPluginBroadcast onBroadcast(message, pluginID, pluginName, text);
  for (const Plugin &plugin : plugins)
    if (&plugin != &callingPlugin)
      calledPlugins += plugin.runCallbackThreaded(onBroadcast);
  return calledPlugins;
}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  const bool insideTell = beginTell(cursor, lastTellPosition);
  cursor.insertText(text, colorFormat(foreground, background));
  endTell(cursor, insideTell);
  lastTellPosition = cursor.position();
}

int ScriptApi::DatabaseClose(string_view databaseID)
{
  auto search = databases.find((string)databaseID);
  if (search == databases.end()) [[unlikely]]
    return -1;

  const int result = search->second.close();
  databases.erase(search);
  return result;
}

int ScriptApi::DatabaseOpen(string_view databaseID, string_view filename, int flags)
{
  auto search = databases.emplace((string)databaseID, filename);
  DatabaseConnection &db = search.first->second;
  if (!search.second)
    return db.isFile(databaseID) ? SQLITE_OK : -6;

  const int result = db.open(flags);
  if (result != SQLITE_OK)
    databases.erase(search.first);

  return result;
}

ApiCode ScriptApi::DoAfter(size_t plugin, double seconds, const QString &text, SendTarget target)
{
  if (seconds < 0.1 || seconds > 86399)
    return ApiCode::TimeInvalid;

  const double ms = seconds * 1000.0;
  const int timerId = startTimer(ms);
  sendQueue[timerId] = {plugin, target, text};
  return ApiCode::OK;
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
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
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
  if (!prop)
    return QVariant();

  return tab()->world.property(prop);
}

optional<string_view> ScriptApi::GetVariable(size_t index, string_view key) const
{
  size_t size;
  const char *variable = client()->getVariable(index, key.data(), key.size(), &size);
  if (!variable)
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

const QString &ScriptApi::GetPluginID(size_t index) const
{
  return plugins.at(index).id();
}

QVariant ScriptApi::GetPluginInfo(string_view pluginID, int infoType) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin || infoType < 0 || infoType > UINT8_MAX) [[unlikely]]
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

QColor ScriptApi::PickColour(const QColor &hint) const
{
  return QColorDialog::getColor(hint, tab());
}

ApiCode ScriptApi::PluginSupports(string_view pluginID, string_view routine) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]]
    return ApiCode::NoSuchPlugin;
  return plugins[index].hasFunction(routine) ? ApiCode::OK : ApiCode::NoSuchRoutine;
}

ApiCode ScriptApi::Send(QByteArrayView view)
{
  echo(QString::fromUtf8(view));
  return SendNoEcho(view);
}

ApiCode ScriptApi::SendNoEcho(QByteArrayView view) const
{
  // run callbacks
  return SendPacket(view);
}

ApiCode ScriptApi::SendPacket(QByteArrayView view) const
{
  if (view.isEmpty()) [[unlikely]]
    return ApiCode::OK;

  QTcpSocket &socket = *tab()->socket;

  if (!socket.isOpen()) [[unlikely]]
    return ApiCode::WorldClosed;

  socket.write(view.constData(), view.size());
  return ApiCode::OK;
}

ApiCode ScriptApi::SetCursor(Qt::CursorShape cursor) const
{
  tab()->ui->area->setCursor(cursor);
  return ApiCode::OK;
}

ApiCode ScriptApi::SetOption(string_view name, const QVariant &variant) const
{
  WorldTab &worldtab = *tab();
  World &world = worldtab.world;
  const char *prop = WorldProperties::canonicalName(name);
  if (!prop) [[unlikely]]
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
