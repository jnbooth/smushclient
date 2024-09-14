#include "scriptapi.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"

QTextCharFormat colorFormat(const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  if (foreground.isValid())
    format.setForeground(QBrush(foreground));
  if (background.isValid())
    format.setBackground(QBrush(background));
  return format;
}

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      cursor(parent->ui->output->document()) {}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  cursor.insertText(text, colorFormat(foreground, background));
  needsNewline = true;
}

ScriptReturnCode ScriptApi::Send(const QByteArrayView &view)
{
  if (view.isEmpty())
    return ScriptReturnCode::OK;

  QTcpSocket &socket = *tab()->socket;

  if (!socket.isOpen())
    return ScriptReturnCode::WorldClosed;

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
  return ScriptReturnCode::OK;
}

void ScriptApi::Tell(const QString &text)
{
  cursor.insertText(text);
  needsNewline = true;
}

void ScriptApi::ensureNewline()
{
  if (!needsNewline)
    return;
  needsNewline = false;
  if (cursor.atBlockStart())
    return;
  cursor.insertBlock();
}

void ScriptApi::insertBlock()
{
  cursor.insertBlock();
  needsNewline = false;
}

std::unordered_map<std::string, std::string> *ScriptApi::getVariableMap(const std::string &pluginID)
{
  auto search = variables.find(pluginID);
  if (search == variables.end())
    return nullptr;
  return search->second;
}

void ScriptApi::printError(const QString &error)
{
  ensureNewline();
  QTextCharFormat errorFormat;
  errorFormat.setForeground(QBrush(tab()->world.getErrorColor()));
  cursor.insertText(error, errorFormat);
  insertBlock();
}

void ScriptApi::setVariableMap(const std::string &pluginID, std::unordered_map<std::string, std::string> *variableMap)
{
  variables[pluginID] = variableMap;
}

bool ScriptApi::unsetVariableMap(const std::string &pluginID, std::unordered_map<std::string, std::string> *variableMap)
{
  auto search = variables.find(pluginID);
  if (search == variables.end() || search->second != variableMap)
    return false;
  variables.erase(pluginID);
  return true;
}
