#pragma once
#include <string>
#include <unordered_map>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QTextCursor>
#include "scriptenums.h"
#include "scriptstate.h"

class World;
class WorldTab;
struct lua_State;

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  static int RGBColourToCode(const QColor &color);
  static QColor RGBCodeToColour(int rgb);
  static void SetClipboard(const QString &text);

  ScriptApi(WorldTab *parent);

  void ColourTell(const QColor &foreground, const QColor &background, const QString &text);
  QVariant GetOption(std::string_view name) const;
  ScriptReturnCode Send(const QByteArrayView &bytes);
  ScriptReturnCode SendNoEcho(const QByteArrayView &bytes) const;
  ScriptReturnCode SetOption(std::string_view name, const QVariant &variant) const;
  void Tell(const QString &text);

  void applyWorld(const World &world);
  void echo(const QString &text);
  void finishNote();
  lua_State *getLuaState(std::string_view pluginID) const;
  void initializeScripts(const QStringList &scripts);
  void printError(const QString &message);
  inline bool runScript(size_t plugin, const QString &script) { return runScript(plugins[plugin], script); }

private:
  QTextCursor cursor;
  QTextCharFormat echoFormat;
  QTextCharFormat errorFormat;
  int lastTellPosition;
  std::vector<ScriptState> plugins;
  std::unordered_map<std::string, size_t> pluginIndices;

  bool handleResult(RunScriptResult result, const ScriptState &plugin);
  bool runScript(ScriptState &plugin, const QString &script);
  inline WorldTab *tab() const { return (WorldTab *)parent(); }
};
