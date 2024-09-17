#pragma once
#include <string>
#include <unordered_map>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QTextCursor>
#include "scriptenums.h"
#include "plugin.h"

class SmushClient;
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
  ApiCode EnableAlias(const QString &label, bool enabled) const;
  ApiCode EnableAliasGroup(const QString &group, bool enabled) const;
  ApiCode EnablePlugin(std::string_view pluginID, bool enabled);
  ApiCode EnableTimer(const QString &label, bool enabled) const;
  ApiCode EnableTimerGroup(const QString &group, bool enabled) const;
  ApiCode EnableTrigger(const QString &label, bool enabled) const;
  ApiCode EnableTriggerGroup(const QString &group, bool enabled) const;
  QVariant GetOption(std::string_view name) const;
  QVariant GetPluginInfo(std::string_view pluginID, uint8_t infoType) const;
  void Hyperlink(const QString &action, const QString &text, const QString &hint, const QColor &foreground, const QColor &background, bool url, bool noUnderline);
  ApiCode IsAlias(const QString &label) const;
  ApiCode IsTimer(const QString &label) const;
  ApiCode IsTrigger(const QString &label) const;
  ApiCode Send(const QByteArrayView &bytes);
  ApiCode SendNoEcho(const QByteArrayView &bytes) const;
  ApiCode SetOption(std::string_view name, const QVariant &variant) const;
  void Tell(const QString &text);

  void applyWorld(const World &world);
  void echo(const QString &text);
  void finishNote();
  const Plugin *getPlugin(std::string_view pluginID) const;
  void initializeScripts(const QStringList &scripts);
  void printError(const QString &message);
  inline bool runScript(size_t plugin, const QString &script) { return runScript(plugins[plugin], script); }

private:
  QTextCursor cursor;
  QTextCharFormat echoFormat;
  QTextCharFormat errorFormat;
  int lastTellPosition;
  std::vector<Plugin> plugins;
  std::unordered_map<std::string, size_t> pluginIndices;

  SmushClient *client() const;
  size_t findPluginIndex(std::string_view pluginID) const;
  bool handleResult(RunScriptResult result, const Plugin &plugin);
  bool runScript(Plugin &plugin, const QString &script);
  inline WorldTab *tab() const { return (WorldTab *)parent(); }
};
