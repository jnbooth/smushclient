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

using std::string;
using std::string_view;

// utils

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

// constructor

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      actionSource(ActionSource::Unknown),
      callbackFilter(),
      cursor(parent->ui->output->document()),
      lastTellPosition(-1)
{
  applyWorld(parent->world);
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
  if (!worldtab) [[unlikely]]
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
  const size_t size = scripts.size();
  callbackFilter.clear();
  plugins.clear();
  plugins.reserve(size);
  pluginIndices.clear();
  pluginIndices.reserve(size);
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
    if (plugin.runScript(*++it))
      callbackFilter.scan(plugin.state());
    else
      plugin.disable();
  }
  OnPluginInstall onInstall;
  sendCallback(onInstall);
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
}

void ScriptApi::printError(const QString &error)
{
  cursor.insertText(error, errorFormat);
  cursor.insertBlock();
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

bool ScriptApi::sendCallback(PluginCallback &callback, const QString &pluginID)
{
  const size_t index = findPluginIndex(pluginID.toStdString());
  if (index == noSuchPlugin) [[unlikely]]
    return false;

  const ActionSource callbackSource = callback.source();

  if (callbackSource == ActionSource::Unknown)
    return plugins[index].runCallback(callback);

  const ActionSource initialSource = actionSource;
  actionSource = callbackSource;
  const bool succeeded = plugins[index].runCallback(callback);
  actionSource = initialSource;
  return succeeded;
}

// protected overrides

void ScriptApi::timerEvent(QTimerEvent *event)
{
  auto search = sendQueue.find(event->timerId());
  if (search == sendQueue.end()) [[unlikely]]
    return;
  const QueuedSend &send = search->second;
  const ActionSource oldSource = actionSource;
  actionSource = ActionSource::TimerFired;
  sendTo(send.plugin, send.target, send.text);
  sendQueue.erase(search);
  actionSource = oldSource;
}

// private methods

void ScriptApi::displayStatusMessage(const QString &status) const
{
  QMainWindow *window = getMainWindow(this);
  if (!window)
    return;

  QStatusBar *statusBar = window->statusBar();
  if (!statusBar)
    return;

  statusBar->showMessage(status);
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

ActionSource ScriptApi::setSource(ActionSource source) noexcept
{
  const ActionSource previousSource = actionSource;
  actionSource = source;
  return previousSource;
}

void ScriptApi::sendTo(size_t plugin, SendTarget target, const QString &text)
{
  switch (target)
  {
  case SendTarget::World:
  case SendTarget::WorldDelay:
  case SendTarget::WorldImmediate:
    tab()->sendCommand(text);
    return;
  case SendTarget::Command:
    tab()->ui->input->setText(text);
    return;
  case SendTarget::Output:
    cursor.insertText(text);
    return;
  case SendTarget::Script:
  case SendTarget::ScriptAfterOmit:
    runScript(plugin, text);
    return;
  case SendTarget::Status:
    displayStatusMessage(text);
    return;
  default:
    return;
  }
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

inline WorldTab *ScriptApi::tab() const { return qobject_cast<WorldTab *>(parent()); }
