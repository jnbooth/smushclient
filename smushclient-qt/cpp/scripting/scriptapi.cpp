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
#include "../bridge/timekeeper.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"
#include "../../spans.h"

using std::string;
using std::string_view;
using std::variant;
using std::chrono::milliseconds;

// Private utils

inline void scrollToEnd(QScrollBar &bar)
{
  bar.setValue(bar.maximum());
}

// Public methods

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      actionSource(ActionSource::Unknown),
      audioChannels(),
      callbackFilter(),
      cursor(parent->ui->output->document()),
      hasLine(false),
      lastTellPosition(-1),
      scrollBar(parent->ui->output->verticalScrollBar()),
      socket(parent->socket),
      whenConnected(QDateTime::currentDateTime())
{
  timekeeper = new Timekeeper(this);
  setLineType(echoFormat, LineType::Input);
  applyWorld(parent->world);
}

void ScriptApi::appendHtml(const QString &html)
{
  flushLine();
  cursor.insertHtml(html);
}

void ScriptApi::appendTell(const QString &text, const QTextCharFormat &format)
{
  if (cursor.position() != lastTellPosition)
  {
    flushLine();
    updateTimestamp();
  }
  cursor.insertText(text, format);
  hasLine = true;
  lastTellPosition = cursor.position();
  scrollToBottom();
}

void ScriptApi::appendText(const QString &text, const QTextCharFormat &format)
{
  flushLine();
  cursor.insertText(text, format);
}

void ScriptApi::appendText(const QString &text)
{
  flushLine();
  cursor.insertText(text);
}

void ScriptApi::applyWorld(const World &world)
{
  QTextCharFormat noteFormat;
  setLineType(noteFormat, LineType::Note);
  noteFormat.setForeground(QBrush(world.getCustomColor()));
  cursor.setCharFormat(noteFormat);
  echoFormat.setForeground(QBrush(world.getEchoTextColour()));
  echoFormat.setBackground(QBrush(world.getEchoBackgroundColour()));
  errorFormat.setForeground(QBrush(world.getErrorColour()));
}

void ScriptApi::echo(const QString &text)
{
  appendText(text, echoFormat);
  startLine();
  scrollToBottom();
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
  const size_t size = scripts.size();
  callbackFilter.clear();
  plugins.clear();
  plugins.reserve(size);
  pluginIndices.clear();
  pluginIndices.reserve(size);
  if (!sendQueue.empty())
  {
    for (const auto &entry : sendQueue)
      killTimer(entry.first);
    sendQueue.clear();
  }
  QString error;
  size_t index = 0;
  for (auto start = scripts.cbegin(), it = start, end = scripts.cend(); it != end; ++it, ++index)
  {
    PluginMetadata metadata{
        .id = *it,
        .index = index,
        .name = *++it,
    };
    pluginIndices[metadata.id.toStdString()] = index;
    Plugin &plugin = plugins.emplace_back(this, std::move(metadata));
    if (!plugin.runScript(*++it))
      continue;
    callbackFilter.scan(plugin.state());
    client()->startTimers(index, *timekeeper);
  }
  OnPluginInstall onInstall;
  sendCallback(onInstall);
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
}

void ScriptApi::printError(const QString &error)
{
  appendText(error, errorFormat);
  startLine();
}

bool ScriptApi::runScript(const QString &pluginID, const QString &script) const
{
  const size_t index = findPluginIndex(pluginID.toStdString());
  if (index == noSuchPlugin) [[unlikely]]
    return false;
  return runScript(index, script);
}

void ScriptApi::sendCallback(PluginCallback &callback)
{
  if (!callbackFilter.includes(callback))
    return;

  const ActionSource callbackSource = callback.source();
  if (callbackSource == ActionSource::Unknown)
  {
    for (const Plugin &plugin : plugins)
      plugin.runCallback(callback);
    return;
  }

  const ActionSource initialSource = actionSource;
  actionSource = callbackSource;

  for (const Plugin &plugin : plugins)
    plugin.runCallback(callback);

  actionSource = initialSource;
}

bool ScriptApi::sendCallback(PluginCallback &callback, size_t index)
{
  const ActionSource callbackSource = callback.source();

  if (callbackSource == ActionSource::Unknown)
    return plugins[index].runCallback(callback);

  const ActionSource initialSource = actionSource;
  actionSource = callbackSource;
  const bool succeeded = plugins[index].runCallback(callback);
  actionSource = initialSource;
  return succeeded;
}

bool ScriptApi::sendCallback(PluginCallback &callback, const QString &pluginID)
{
  const size_t index = findPluginIndex(pluginID.toStdString());
  if (index == noSuchPlugin) [[unlikely]]
    return false;

  return sendCallback(callback, index);
}

void ScriptApi::sendTo(size_t plugin, SendTarget target, const QString &text)
{
  if (text.isEmpty())
    return;
  switch (target)
  {
  case SendTarget::World:
  case SendTarget::WorldDelay:
  case SendTarget::WorldImmediate:
    Send(text);
    return;
  case SendTarget::Command:
    tab()->ui->input->setText(text);
    return;
  case SendTarget::Output:
    appendText(text);
    startLine();
    scrollToBottom();
    return;
  case SendTarget::Script:
  case SendTarget::ScriptAfterOmit:
    runScript(plugin, text);
    return;
  case SendTarget::Status:
    SetStatus(text);
    return;
  default:
    return;
  }
}

void ScriptApi::setOpen(bool open) const
{
  timekeeper->setOpen(open);
}

ActionSource ScriptApi::setSource(ActionSource source) noexcept
{
  const ActionSource previousSource = actionSource;
  actionSource = source;
  return previousSource;
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

  if (neighbor)
    window->stackUnder(neighbor);
  else if (drawsUnderneath)
    window->stackUnder(tab()->ui->outputBorder);
}

void ScriptApi::startLine()
{
  if (hasLine) [[unlikely]]
    cursor.insertBlock();
  else
    hasLine = true;
}

void ScriptApi::updateTimestamp()
{
  setTimestamp(cursor);
}

// protected overrides

void ScriptApi::timerEvent(QTimerEvent *event)
{
  const int id = event->timerId();
  killTimer(id);
  auto search = sendQueue.find(id);
  if (search == sendQueue.end()) [[unlikely]]
    return;
  const QueuedSend &send = search->second;
  const ActionSource oldSource = actionSource;
  actionSource = ActionSource::TimerFired;
  sendTo(send.plugin, send.target, send.text);
  actionSource = oldSource;
}

// Private methods

AudioChannel &ScriptApi::getAudioChannel(size_t index)
{
  if (index)
    return audioChannels[index - 1];
  for (AudioChannel &channel : audioChannels)
    if (!channel.isPlaying())
      return channel;
  return audioChannels[0];
}

inline SmushClient *ScriptApi::client() const
{
  return &tab()->client;
}

DatabaseConnection *ScriptApi::findDatabase(string_view databaseID)
{
  auto search = databases.find((string)databaseID);
  if (search == databases.end()) [[unlikely]]
    return nullptr;
  return &search->second;
}

size_t ScriptApi::findPluginIndex(const string &pluginID) const
{
  auto search = pluginIndices.find(pluginID);
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

void ScriptApi::flushLine()
{
  if (hasLine) [[unlikely]]
  {
    hasLine = false;
    cursor.insertBlock();
  }
}

void ScriptApi::scrollToBottom() const
{
  scrollToEnd(*scrollBar);
}

inline WorldTab *ScriptApi::tab() const { return qobject_cast<WorldTab *>(parent()); }
