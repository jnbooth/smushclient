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
  if (!variant.canConvert<QString>())
    return QColor();
  QString colorName = variant.toString();
  return colorName.isEmpty() ? QColor::fromRgb(0, 0, 0, 0) : QColor::fromString(colorName);
}

ScriptApi::ScriptApi(WorldTab *parent)
    : QObject(parent),
      cursor(parent->ui->output->document()) {}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  cursor.insertText(text, colorFormat(foreground, background));
  needsNewline = true;
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

inline ScriptReturnCode applyWorld(WorldTab &worldtab)
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
    return applyWorld(worldtab);

  switch (property.typeId())
  {
  case QMetaType::QColor:
    if (QColor color = getColorFromVariant(variant); color.isValid() && world.setProperty(prop, color))
      return applyWorld(worldtab);
  case QMetaType::QVariantHash:
    if (isEmptyList(variant) && world.setProperty(prop, QVariantHash()))
      return applyWorld(worldtab);
  case QMetaType::QVariantMap:
    if (isEmptyList(variant) && world.setProperty(prop, QVariantMap()))
      return applyWorld(worldtab);
  }

  return ScriptReturnCode::OptionOutOfRange;
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
