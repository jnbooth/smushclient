#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QTextCursor>
#include "hotspot.h"
#include "miniwindow.h"
#include "plugin.h"
#include "scriptenums.h"

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
  ApiCode WindowAddHotspot(
      std::string_view pluginID,
      std::string_view windowName,
      std::string_view hotspotID,
      const QRect &geometry,
      Hotspot::Callbacks &&callbacks,
      const QString &tooltip,
      Qt::CursorShape cursor,
      bool trackHover);
  ApiCode WindowCreate(
      std::string_view windowName,
      const QPoint &location,
      const QSize &size,
      MiniWindow::Position position,
      MiniWindow::Flags flags,
      const QColor &fill);
  ApiCode WindowDeleteHotspot(
      std::string_view windowName,
      std::string_view hotspotID) const;
  ApiCode WindowPosition(
      std::string_view windowName,
      const QPoint &location,
      MiniWindow::Position position,
      MiniWindow::Flags flags) const;
  ApiCode WindowResize(std::string_view windowName, const QSize &size, const QColor &fill) const;
  ApiCode WindowSetZOrder(std::string_view windowName, int zOrder);

  void applyWorld(const World &world);
  void echo(const QString &text);
  void finishNote();
  const Plugin *getPlugin(std::string_view pluginID) const;
  void initializeScripts(const QStringList &scripts);
  void printError(const QString &message);
  inline bool runScript(size_t plugin, const QString &script) const
  {
    return plugins[plugin].runScript(script);
  }
  void stackWindow(std::string_view windowName, MiniWindow *window) const;

private:
  QTextCursor cursor;
  QTextCharFormat echoFormat;
  QTextCharFormat errorFormat;
  int lastTellPosition;
  std::vector<Plugin> plugins;
  std::unordered_map<std::string, size_t> pluginIndices;
  std::unordered_map<std::string, MiniWindow *> windows;

  SmushClient *client() const;
  size_t findPluginIndex(std::string_view pluginID) const;
  MiniWindow *findWindow(std::string_view windowName) const;
  inline WorldTab *tab() const { return (WorldTab *)parent(); }
};
