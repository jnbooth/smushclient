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
#include "../ui/components/mudscrollbar.h"
#include "../ui/components/mudstatusbar.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"
#include "../../spans.h"

using std::string;
using std::string_view;
using std::variant;
using std::chrono::milliseconds;

// Private utils

QMainWindow *getMainWindow(const QObject *obj)
{
  if (!obj)
    return nullptr;

  QObject *parent = obj->parent();
  if (!parent)
    return nullptr;

  QMainWindow *window = qobject_cast<QMainWindow *>(parent);
  if (window)
    return window;

  return getMainWindow(parent);
}

// Public methods

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      actionSource(ActionSource::Unknown),
      audioChannels{
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
          AudioChannel(),
      },
      callbackFilter(),
      cursor(parent->ui->output->document()),
      databases(),
      doNaws(false),
      doesNaws(false),
      echoFormat(),
      echoOnSameLine(false),
      errorFormat(),
      hasLine(false),
      indentNext(false),
      indentText(),
      lastTellPosition(-1),
      noteFormat(),
      plugins(),
      pluginIndices(),
      scrollBar(parent->ui->output->verticalScrollBar()),
      sendQueue(),
      socket(parent->socket),
      statusBar(qobject_cast<MudStatusBar *>(getMainWindow(parent)->statusBar())),
      suppressEcho(false),
      whenConnected(QDateTime::currentDateTime()),
      windows()
{
  timekeeper = new Timekeeper(this);
  setLineType(echoFormat, LineType::Input);
  setLineType(noteFormat, LineType::Note);
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
  doNaws = world.getNaws();
  echoOnSameLine = world.getKeepCommandsOnSameLine();
  if (world.getNoEchoOff())
    suppressEcho = false;

  indentText = QStringLiteral(" ").repeated(world.getIndentParas());
  echoFormat.setForeground(QBrush(world.getEchoTextColour()));
  echoFormat.setBackground(QBrush(world.getEchoBackgroundColour()));
  errorFormat.setForeground(QBrush(world.getErrorColour()));
  noteFormat.setForeground(QBrush(world.getNoteTextColour()));
}

void ScriptApi::echo(const QString &text)
{
  if (suppressEcho) [[unlikely]]
    return;
  if (echoOnSameLine)
  {
    cursor.insertText(text, echoFormat);
    return;
  }
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

void ScriptApi::initializePlugins(const rust::Vec<PluginPack> &pack)
{
  if (!windows.empty())
  {
    for (const auto &entry : windows)
      delete entry.second;
    windows.clear();
  }
  const size_t size = pack.size();
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
  for (auto start = pack.cbegin(), it = start, end = pack.cend(); it != end; ++it, ++index)
  {
    PluginMetadata metadata{
        .id = it->id.toStdString(),
        .index = index,
        .name = it->name.toStdString(),
    };
    pluginIndices[metadata.id] = index;
    Plugin &plugin = plugins.emplace_back(this, std::move(metadata));
    if (!plugin.runScript(string_view(reinterpret_cast<const char *>(it->scriptData), it->scriptSize)))
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
  scrollToBottom();
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
  switch (callback.id())
  {
  case OnPluginConnect::ID:
    statusBar->setConnected(true);
    break;

  case OnPluginDisconnect::ID:
    doesNaws = false;
    statusBar->setConnected(false);
    break;

  case OnPluginWorldOutputResized::ID:
    for (const auto &window : windows)
      window.second->updatePosition();
    sendNaws();
    break;
  }

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

void ScriptApi::sendNaws() const
{
  if (!doesNaws || !doNaws)
    return;
  MudBrowser *browser = tab()->ui->output;
  const QFontMetrics metrics = browser->fontMetrics();
  const QMargins margins = browser->contentsMargins();
  const int advance = metrics.horizontalAdvance(QStringLiteral("0123456789"));
  SendPacket(encodeNaws(
      (browser->width() - margins.left() - margins.right() - scrollBar->width()) * 10 / advance,
      (browser->height() - margins.top() - margins.bottom()) / metrics.lineSpacing() - 4));
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
    appendText(text, noteFormat);
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

void ScriptApi::setNawsEnabled(bool enabled)
{
  doesNaws = enabled;
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

void ScriptApi::setSuppressEcho(bool suppress) noexcept
{
  suppressEcho = suppress;
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
  const WindowCompare compare{-window->getZOrder(), windowName};
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
  {
    cursor.insertBlock();
    indentNext = !indentText.isEmpty();
  }
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
  if (!hasLine) [[likely]]
    return;

  hasLine = false;
  cursor.insertBlock();

  if (!indentNext) [[likely]]
    return;

  indentNext = false;
  cursor.insertText(indentText);
}

void ScriptApi::scrollToBottom() const
{
  scrollBar->toEnd();
}

inline WorldTab *ScriptApi::tab() const { return qobject_cast<WorldTab *>(parent()); }
