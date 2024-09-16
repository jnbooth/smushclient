#include <QtGui/QGuiApplication>
#include <QtGui/QClipboard>
#include "scriptapi.h"
#include "../ui/worldtab.h"
#include "../ui/ui_worldtab.h"
#include "worldproperties.h"

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

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      cursor(parent->ui->output->document()),
      lastTellPosition(-1)
{
  applyWorld(parent->world);
}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  const bool insideTell = beginTell(cursor, lastTellPosition);
  cursor.insertText(text, colorFormat(foreground, background));
  endTell(cursor, insideTell);
  lastTellPosition = cursor.position();
}

QVariant ScriptApi::GetOption(const std::string &name) const
{
  const char *prop = WorldProperties::canonicalName(name);
  if (prop == nullptr)
    return QVariant();

  return tab()->world.property(prop);
}

ScriptReturnCode ScriptApi::Send(const QByteArrayView &view)
{
  echo(QString::fromUtf8(view));
  return SendNoEcho(view);
}

ScriptReturnCode ScriptApi::SendNoEcho(const QByteArrayView &view)
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

inline ScriptReturnCode updateWorld(WorldTab &worldtab)
{
  return worldtab.updateWorld() ? ScriptReturnCode::OK : ScriptReturnCode::OptionOutOfRange;
}

ScriptReturnCode ScriptApi::SetOption(const std::string &name, const QVariant &variant)
{
  WorldTab &worldtab = *tab();
  World &world = worldtab.world;
  const char *prop = WorldProperties::canonicalName(name);
  if (prop == nullptr)
    return ScriptReturnCode::UnknownOption;
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

  return ScriptReturnCode::OptionOutOfRange;
}

void ScriptApi::Tell(const QString &text)
{
  const bool insideTell = beginTell(cursor, lastTellPosition);
  cursor.insertText(text);
  endTell(cursor, insideTell);
  lastTellPosition = cursor.position();
}

void ScriptApi::applyWorld(const World &world)
{
  QTextCharFormat noteFormat;
  noteFormat.setForeground(QBrush(world.getCustomColor()));
  cursor.setCharFormat(noteFormat);
  echoFormat.setForeground(QBrush(world.getEchoTextColour()));
  echoFormat.setBackground(QBrush(world.getEchoBackgroundColour()));
  errorFormat.setForeground(QBrush(world.getErrorColour()));
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

std::unordered_map<std::string, std::string> *ScriptApi::getVariableMap(const std::string &pluginID)
{
  auto search = variables.find(pluginID);
  if (search == variables.end())
    return nullptr;
  return search->second;
}

void ScriptApi::printError(const QString &error)
{
  cursor.insertText(error, errorFormat);
  cursor.insertBlock();
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
