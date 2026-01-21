#include "scriptapi.h"
#include "../bridge/timekeeper.h"
#include "../bytes.h"
#include "../spans.h"
#include "../timer_map.h"
#include "../ui/mudstatusbar/mudstatusbar.h"
#include "../ui/notepad.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "miniwindow.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include "sqlite3.h"
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtGui/QTextBlock>
#include <QtWidgets/QStatusBar>

using std::string;
using std::string_view;
using std::chrono::milliseconds;
using std::chrono::seconds;

// Public methods

ScriptApi::ScriptApi(const SmushClient* client,
                     QAbstractSocket* socket,
                     MudBrowser* output,
                     MudStatusBar* statusBar,
                     Notepads* notepads,
                     WorldTab* parent)
  : QObject(parent)
  , timekeeper(new Timekeeper(client, this))
  , client(client)
  , cursor(output->document())
  , notepads(notepads)
  , scrollBar(output->verticalScrollBar())
  , sendQueue(new TimerMap<SendRequest>(this, &ScriptApi::finishQueuedSend))
  , socket(socket)
  , statusBar(statusBar)
  , tab(parent)
  , whenConnected(QDateTime::currentDateTime())
{
  timekeeper->beginPolling(milliseconds(seconds{ 60 }));
  setLineType(echoFormat, LineType::Input);
  setLineType(noteFormat, LineType::Note);
}

ScriptApi::~ScriptApi()
{
  delete statusBar;
}

void
ScriptApi::appendHtml(const QString& html)
{
  flushLine();
  cursor.insertHtml(html);
}

void
ScriptApi::appendTell(const QString& text, const QTextCharFormat& format)
{
  if (text.isEmpty()) {
    return;
  }
  if (cursor.position() != lastTellPosition) {
    flushLine();
    updateTimestamp();
  }
  cursor.insertText(text, format);
  hasLine = true;
  lastTellPosition = cursor.position();
  if (logNotes) {
  }
}

void
ScriptApi::appendText(const QString& text, const QTextCharFormat& format)
{
  flushLine();
  cursor.insertText(text, format);
}

void
ScriptApi::appendText(const QString& text)
{
  appendText(text, noteFormat);
}

void
ScriptApi::applyWorld(const World& world)
{
  doNaws = world.getNaws();
  logNotes = world.getLogNotes();
  echoOnSameLine = world.getKeepCommandsOnSameLine();
  echoInput = world.getDisplayMyInput();

  if (world.getNoEchoOff()) {
    suppressEcho = false;
  }

  indentText = QStringLiteral(" ").repeated(world.getIndentParas());
  echoFormat.setForeground(world.getEchoColour());
  echoFormat.setBackground(world.getEchoBackgroundColour());
  errorFormat.setForeground(world.getErrorTextColour());
  errorFormat.setBackground(world.getErrorBackgroundColour());
  noteFormat.setForeground(world.getNoteTextColour());
  noteFormat.setBackground(world.getNoteBackgroundColour());

  if (worldScriptIndex == noSuchPlugin) {
    return;
  }

  if (world.getEnableScripts()) {
    plugins[worldScriptIndex].enable();
  } else {
    plugins[worldScriptIndex].disable();
  }
}

void
ScriptApi::echo(const QString& text)
{
  if (suppressEcho) [[unlikely]] {
    return;
  }
  if (echoOnSameLine) {
    cursor.insertText(text, echoFormat);
    return;
  }
  appendText(text, echoFormat);
  startLine();
}

void
ScriptApi::finishNote()
{
  lastTellPosition = -1;
}

const Plugin*
ScriptApi::getPlugin(string_view pluginID) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]] {
    return nullptr;
  }
  return &plugins[index];
}

void
ScriptApi::handleSendRequest(const SendRequest& request)
{
  if (request.text.isEmpty()) {
    return;
  }
  switch (request.sendTo) {
    case SendTarget::World:
    case SendTarget::WorldDelay:
    case SendTarget::Execute:
    case SendTarget::Speedwalk:
    case SendTarget::WorldImmediate: {
      SendFlags flags;
      if (request.echo) {
        flags |= SendFlag::Echo;
      }
      if (request.log) {
        flags |= SendFlag::Log;
      }
      sendToWorld(request.text, flags);
    }
      return;
    case SendTarget::Command:
      tab->ui->input->setText(request.text);
      return;
    case SendTarget::Output:
      appendText(request.text, noteFormat);
      startLine();
      return;
    case SendTarget::Status:
      SetStatus(request.text);
      return;
    case SendTarget::NotepadNew:
      notepads->pad()->insertPlainText(request.text);
      return;
    case SendTarget::NotepadAppend: {
      QTextCursor notepadCursor =
        notepads->pad(request.destination)->textCursor();
      if (!notepadCursor.atBlockStart()) {
        notepadCursor.insertBlock();
      }
      notepadCursor.insertText(request.text);
      return;
    }
    case SendTarget::NotepadReplace:
      notepads->pad(request.destination)->setPlainText(request.text);
      return;
    case SendTarget::Log:
    case SendTarget::Variable:
      return;
    case SendTarget::Script:
    case SendTarget::ScriptAfterOmit:
      const QByteArray utf8 = request.text.toUtf8();
      runScript(request.plugin, string_view(utf8.data(), utf8.size()));
      return;
  }
}

void
ScriptApi::initializePlugins()
{
  const rust::Vec<PluginPack> pack = client->resetPlugins();
  worldScriptIndex = noSuchPlugin;
  if (!windows.empty()) {
    for (const auto& entry : windows) {
      delete entry.second;
    }
    windows.clear();
  }
  const size_t size = pack.size();
  callbackFilter.clear();
  plugins.clear();
  plugins.reserve(size);
  pluginIndices.clear();
  pluginIndices.reserve(size);
  sendQueue->clear();
  QString error;
  size_t index = 0;
  for (auto start = pack.cbegin(), it = start, end = pack.cend(); it != end;
       ++it, ++index) {
    PluginMetadata metadata(*it, index);
    if (metadata.id.empty()) {
      worldScriptIndex = index;
    }
    pluginIndices[metadata.id] = index;
    Plugin& plugin = plugins.emplace_back(this, *it, index);
    const string& pluginId = plugin.id();
    if (pluginId.empty()) {
      worldScriptIndex = index;
    }
    pluginIndices[pluginId] = index;
    if (plugin.install(*it)) {
      callbackFilter.scan(plugin.state());
      client->startTimers(index, *timekeeper);
    }
  }
  OnPluginInstall onInstall;
  sendCallback(onInstall);
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
}

ApiCode
ScriptApi::playFileRaw(const QString& path) const
{
  const QByteArray utf8 = path.toUtf8();
  return client->playFileRaw(bytes::slice(utf8));
}

void
ScriptApi::reinstallPlugin(size_t index)
{
  const PluginPack pack = client->plugin(index);
  const string pluginId(pack.id.data(), pack.id.size());
  if (!windows.empty()) {
    for (auto it = windows.begin(); it != windows.end();) {
      if (MiniWindow* window = it->second; window->getPluginId() == pluginId) {
        delete window;
        it = windows.erase(it);
      } else {
        ++it;
      }
    }
  }
  Plugin& plugin = plugins[index];
  plugin.updateMetadata(pack, index);
  plugin.reset();
  if (!plugin.install(pack)) {
    return;
  }
  callbackFilter.scan(plugin.state());
  client->startTimers(index, *timekeeper);
  OnPluginInstall onInstall;
  sendCallback(onInstall, index);
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
}

void
ScriptApi::printError(const QString& message)
{
  appendText(message, errorFormat);
  startLine();
}

void
ScriptApi::reloadWorldScript(const QString& worldScriptPath)
{
  if (worldScriptIndex == noSuchPlugin) {
    return;
  }
  Plugin& worldPlugin = plugins[worldScriptIndex];
  worldPlugin.reset();

  if (worldScriptPath.isEmpty()) {
    return;
  }

  if (!worldPlugin.runFile(worldScriptPath)) {
    worldPlugin.disable();
  }
}

void
ScriptApi::resetAllTimers()
{
  sendQueue->clear();
  client->startAllTimers(*timekeeper);
}

void
ScriptApi::sendCallback(PluginCallback& callback)
{
  switch (callback.id()) {
    case OnPluginWorldOutputResized::ID:
      for (const auto& window : windows) {
        window.second->updatePosition();
      }
      sendNaws();
      break;
    default:
      break;
  }

  if (!callbackFilter.includes(callback)) {
    return;
  }

  const ActionSource callbackSource = callback.source();
  if (callbackSource == ActionSource::Unknown) {
    for (const Plugin& plugin : plugins) {
      plugin.runCallback(callback);
    }
    return;
  }

  const ActionSource initialSource = actionSource;
  actionSource = callbackSource;

  for (const Plugin& plugin : plugins) {
    plugin.runCallback(callback);
  }

  actionSource = initialSource;
}

bool
ScriptApi::sendCallback(PluginCallback& callback, size_t plugin)
{
  const ActionSource callbackSource = callback.source();

  if (callbackSource == ActionSource::Unknown) {
    return plugins[plugin].runCallback(callback);
  }

  const ActionSource initialSource = actionSource;
  actionSource = callbackSource;
  const bool succeeded = plugins[plugin].runCallback(callback);
  actionSource = initialSource;
  return succeeded;
}

bool
ScriptApi::sendCallback(PluginCallback& callback, const QString& pluginID)
{
  const size_t index = findPluginIndex(pluginID.toStdString());
  if (index == noSuchPlugin) [[unlikely]] {
    return false;
  }

  return sendCallback(callback, index);
}

void
ScriptApi::sendNaws()
{
  if (!doesNaws || !doNaws) {
    return;
  }
  MudBrowser* browser = tab->ui->output;
  const QFontMetrics metrics = browser->fontMetrics();
  const QMargins margins = browser->contentsMargins();
  const int advance = metrics.horizontalAdvance(QStringLiteral("0123456789"));
  const QSize viewport = browser->maximumViewportSize();
  SendPacket(ffi::encodeNaws(
    (viewport.width() - margins.left() - margins.right()) * 10 / advance,
    (viewport.height() - margins.top() - margins.bottom()) /
        metrics.lineSpacing() -
      4));
}

void
ScriptApi::setNawsEnabled(bool enabled)
{
  doesNaws = enabled;
}

void
ScriptApi::setOpen(bool open) const
{
  timekeeper->setOpen(open);
}

ActionSource
ScriptApi::setSource(ActionSource source) noexcept
{
  const ActionSource previousSource = actionSource;
  actionSource = source;
  return previousSource;
}

void
ScriptApi::setSuppressEcho(bool suppress) noexcept
{
  suppressEcho = suppress;
}

struct WindowCompare
{
  int64_t zOrder = 0;
  string_view name;
  std::strong_ordering operator<=>(const WindowCompare&) const = default;
};

void
ScriptApi::stackWindow(string_view windowName, MiniWindow* window) const
{
  const bool drawsUnderneath = window->drawsUnderneath();
  const WindowCompare compare{ .zOrder = -window->getZOrder(),
                               .name = windowName };
  MiniWindow* neighbor = nullptr;
  WindowCompare neighborCompare;

  for (const auto& entry : windows) {
    if (entry.second == window ||
        entry.second->drawsUnderneath() != drawsUnderneath) {
      continue;
    }
    WindowCompare entryCompare{ .zOrder = entry.second->getZOrder(),
                                .name = entry.first };
    if (entryCompare > compare &&
        ((neighbor == nullptr) || entryCompare < neighborCompare)) {
      neighbor = entry.second;
      neighborCompare = entryCompare;
    }
  }

  if (neighbor != nullptr) {
    window->stackUnder(neighbor);
  } else if (drawsUnderneath) {
    window->stackUnder(tab->ui->outputBorder);
  }
}

int
ScriptApi::startLine()
{
  if (hasLine) [[unlikely]] {
    cursor.insertBlock();
    indentNext = !indentText.isEmpty();
  } else {
    hasLine = true;
  }
  return cursor.position();
}

void
ScriptApi::updateTimestamp()
{
  setTimestamp(cursor);
}

// Private methods

DatabaseConnection*
ScriptApi::findDatabase(string_view databaseID)
{
  auto search = databases.find(databaseID);
  if (search == databases.end()) [[unlikely]] {
    return nullptr;
  }
  return &search->second;
}

size_t
ScriptApi::findPluginIndex(const string& pluginID) const
{
  auto search = pluginIndices.find(pluginID);
  if (search == pluginIndices.end()) [[unlikely]] {
    return noSuchPlugin;
  }
  return search->second;
}

MiniWindow*
ScriptApi::findWindow(string_view windowName) const
{
  auto search = windows.find(windowName);
  if (search == windows.end()) [[unlikely]] {
    return nullptr;
  }
  return search->second;
}

bool
ScriptApi::finishQueuedSend(const SendRequest& request)
{
  const ActionSource oldSource = actionSource;
  actionSource = ActionSource::TimerFired;
  handleSendRequest(request);
  actionSource = oldSource;
  return true;
}

void
ScriptApi::flushLine()
{
  if (!hasLine) [[likely]] {
    return;
  }

  hasLine = false;
  cursor.insertBlock();

  if (!indentNext) [[likely]] {
    return;
  }

  indentNext = false;
  cursor.insertText(indentText);
}

void
ScriptApi::insertBlock()
{
  if (logNotes && lastTellPosition >= lastLinePosition) {
    client->logNote(cursor.block().text());
  }
  cursor.insertBlock();
  lastLinePosition = cursor.position();
}

ApiCode
ScriptApi::sendToWorld(QByteArray& bytes, const QString& text, SendFlags flags)
{
  OnPluginSend onSend(&bytes);
  sendCallback(onSend);
  if (onSend.discarded()) {
    return ApiCode::OK;
  }

  if (echoInput && flags.testFlag(SendFlag::Echo)) {
    echo(text);
  }

  lastCommandSent = bytes;

  if (flags.testFlag(SendFlag::Log)) {
    client->logInput(text);
  }

  bytes.append("\r\n");

  const qsizetype size = bytes.size();
  totalLinesSent += bytes.count('\n');
  totalPacketsSent += 1;
  if (socket->write(bytes.constData(), size) == -1) [[unlikely]] {
    return ApiCode::WorldClosed;
  }
  bytes.truncate(size - 2);

  OnPluginSent onSent(&bytes);
  sendCallback(onSent);
  return ApiCode::OK;
}
