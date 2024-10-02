#pragma once
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <QtCore/QMargins>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QTextCursor>
#include "databaseconnection.h"
#include "hotspot.h"
#include "miniwindow.h"
#include "plugin.h"
#include "scriptenums.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

#define SCRIPTING_VERSION "5.07"
constexpr size_t noSuchPlugin = SIZE_T_MAX;

class ImageFilter;
class SmushClient;
class World;
class WorldTab;
struct lua_State;

struct QueuedSend
{
  size_t plugin;
  SendTarget target;
  QString text;
};

class ScriptApi : public QObject
{
public:
  static void SetClipboard(const QString &text);

  ScriptApi(WorldTab *parent);

  int BroadcastPlugin(size_t pluginIndex, int message, std::string_view text) const;
  void ColourTell(const QColor &foreground, const QColor &background, const QString &text);
  int DatabaseClose(std::string_view databaseID);
  int DatabaseOpen(std::string_view databaseID, std::string_view filename, int flags);
  ApiCode DoAfter(size_t plugin, double seconds, const QString &text, SendTarget target);
  ApiCode EnableAlias(const QString &label, bool enabled) const;
  ApiCode EnableAliasGroup(const QString &group, bool enabled) const;
  ApiCode EnablePlugin(std::string_view pluginID, bool enabled);
  ApiCode EnableTimer(const QString &label, bool enabled) const;
  ApiCode EnableTimerGroup(const QString &group, bool enabled) const;
  ApiCode EnableTrigger(const QString &label, bool enabled) const;
  ApiCode EnableTriggerGroup(const QString &group, bool enabled) const;
  QVariant FontInfo(const QFont &font, int infoType) const;
  QVariant GetInfo(int infoType) const;
  QVariant GetOption(std::string_view name) const;
  std::optional<std::string_view> GetVariable(size_t pluginIndex, std::string_view key) const;
  std::optional<std::string_view> GetVariable(std::string_view pluginID, std::string_view key) const;
  const QString &GetPluginID(size_t pluginIndex) const;
  QVariant GetPluginInfo(std::string_view pluginID, int infoType) const;
  void Hyperlink(
      const QString &action,
      const QString &text,
      const QString &hint,
      const QColor &foreground,
      const QColor &background,
      bool url,
      bool noUnderline);
  ApiCode IsAlias(const QString &label) const;
  ApiCode IsTimer(const QString &label) const;
  ApiCode IsTrigger(const QString &label) const;
  QColor PickColour(const QColor &hint) const;
  ApiCode PluginSupports(std::string_view pluginID, std::string_view routine) const;
  ApiCode Send(QByteArrayView bytes);
  ApiCode SendNoEcho(QByteArrayView bytes) const;
  ApiCode SendPacket(QByteArrayView bytes) const;
  ApiCode SetCursor(Qt::CursorShape cursor) const;
  ApiCode SetOption(std::string_view name, const QVariant &variant) const;
  bool SetVariable(size_t pluginIndex, std::string_view key, std::string_view value) const;
  void Tell(const QString &text);
  ApiCode TextRectangle(
      const QRect &rect,
      int borderOffset,
      const QColor &borderColor,
      int borderWidth,
      const QBrush &outsideFill) const;
  ApiCode WindowAddHotspot(
      size_t pluginIndex,
      std::string_view windowName,
      std::string_view hotspotID,
      const QRect &geometry,
      Hotspot::Callbacks &&callbacks,
      const QString &tooltip,
      Qt::CursorShape cursor,
      Hotspot::Flags flags) const;
  ApiCode WindowCreate(
      size_t pluginIndex,
      std::string_view windowName,
      const QPoint &location,
      const QSize &size,
      MiniWindow::Position position,
      MiniWindow::Flags flags,
      const QColor &fill);
  ApiCode WindowDeleteHotspot(std::string_view windowName, std::string_view hotspotID) const;
  ApiCode WindowDrawImage(
      std::string_view windowName,
      std::string_view imageID,
      const QRectF &rect,
      MiniWindow::DrawImageMode mode,
      const QRectF &sourceRect) const;
  ApiCode WindowDrawImageAlpha(
      std::string_view windowName,
      std::string_view imageID,
      const QRectF &rect,
      qreal opacity,
      const QPointF origin) const;
  ApiCode WindowEllipse(
      std::string_view windowName,
      const QRectF &rect,
      const QPen &pen,
      const QBrush &brush = QBrush()) const;
  ApiCode WindowFilter(
      std::string_view windowName,
      const ImageFilter &filter,
      const QRect &rect = QRect()) const;
  ApiCode WindowFont(
      std::string_view windowName,
      std::string_view fontID,
      const QString &fontName,
      qreal pointSize,
      bool bold,
      bool italic,
      bool underline,
      bool strikeout,
      QFont::StyleHint hint) const;
  QVariant WindowFontInfo(
      std::string_view windowName,
      std::string_view fontID,
      int infoType) const;
  ApiCode WindowFrame(
      std::string_view windowName,
      const QRectF &rect,
      const QColor &color1,
      const QColor &color2) const;
  ApiCode WindowGradient(
      std::string_view windowName,
      const QRectF &rect,
      const QColor &color1,
      const QColor &color2,
      Qt::Orientation direction) const;
  QVariant WindowHotspotInfo(
      std::string_view windowName,
      std::string_view hotspotID,
      int infoType) const;
  ApiCode WindowImageFromWindow(
      std::string_view windowName,
      std::string_view imageID,
      std::string_view sourceWindow) const;
  QVariant WindowInfo(std::string_view windowName, int infoType) const;
  ApiCode WindowInvert(std::string_view windowName, const QRect &rect) const;
  ApiCode WindowLine(
      std::string_view windowName,
      const QLineF &line,
      const QPen &pen) const;
  ApiCode WindowLoadImage(
      std::string_view windowName,
      std::string_view imageID,
      const QString &filename) const;
  QVariant WindowMenu(
      std::string_view windowName,
      const QPoint &location,
      std::string_view menuString) const;
  ApiCode WindowMoveHotspot(
      std::string_view windowName,
      std::string_view hotspotID,
      const QRect &geometry) const;
  ApiCode WindowPolygon(
      std::string_view windowName,
      const QPolygonF &polygon,
      const QPen &pen,
      const QBrush &brush,
      bool close,
      Qt::FillRule fillRule) const;
  ApiCode WindowPosition(
      std::string_view windowName,
      const QPoint &location,
      MiniWindow::Position position,
      MiniWindow::Flags flags) const;
  ApiCode WindowRect(
      std::string_view windowName,
      const QRectF &rect,
      const QPen &pen,
      const QBrush &brush = QBrush()) const;
  ApiCode WindowRoundedRect(
      std::string_view windowName,
      const QRectF &rect,
      qreal xRadius,
      qreal yRadius,
      const QPen &pen,
      const QBrush &brush = QBrush()) const;
  ApiCode WindowResize(std::string_view windowName, const QSize &size, const QColor &fill) const;
  ApiCode WindowSetZOrder(std::string_view windowName, int zOrder) const;
  ApiCode WindowShow(std::string_view windowName, bool show) const;
  qreal WindowText(
      std::string_view windowName,
      std::string_view fontID,
      const QString &text,
      const QRectF &rect,
      const QColor &color) const;
  int WindowTextWidth(
      std::string_view windowName,
      std::string_view fontID,
      const QString &text) const;
  ApiCode WindowUpdateHotspot(
      size_t pluginIndex,
      std::string_view windowName,
      std::string_view hotspotID,
      Hotspot::CallbacksPartial &&callbacks) const;
  ApiCode WindowUnloadImage(
      std::string_view windowName,
      std::string_view windowID) const;
  ApiCode WindowUnloadFont(std::string_view windowName, std::string_view fontID) const;

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
  bool runScript(const QString &pluginID, const QString &script) const;
  void sendCallback(PluginCallback &callback);
  bool sendCallback(PluginCallback &callback, const QString &pluginID);
  void sendTo(size_t plugin, SendTarget target, const QString &text);
  ActionSource setSource(ActionSource source) noexcept;
  void stackWindow(std::string_view windowName, MiniWindow *window) const;

  inline constexpr std::vector<Plugin>::const_iterator cbegin() const noexcept
  {
    return plugins.cbegin();
  }
  inline constexpr std::vector<Plugin>::const_iterator begin() const noexcept
  {
    return plugins.begin();
  }
  inline constexpr std::vector<Plugin>::const_iterator cend() const noexcept
  {
    return plugins.cend();
  }
  inline constexpr std::vector<Plugin>::const_iterator end() const noexcept
  {
    return plugins.end();
  }

protected:
  void timerEvent(QTimerEvent *event) override;

private:
  ActionSource actionSource;
  CallbackFilter callbackFilter;
  QTextCursor cursor;
  std::unordered_map<std::string, DatabaseConnection> databases;
  QTextCharFormat echoFormat;
  QTextCharFormat errorFormat;
  int lastTellPosition;
  std::vector<Plugin> plugins;
  std::unordered_map<std::string, size_t> pluginIndices;
  std::unordered_map<int, QueuedSend> sendQueue;
  std::unordered_map<std::string, MiniWindow *> windows;

  SmushClient *client() const;
  void displayStatusMessage(const QString &status) const;
  DatabaseConnection *findDatabase(const std::string_view databaseID);
  size_t findPluginIndex(const std::string &pluginID) const;
  inline size_t findPluginIndex(const std::string_view pluginID) const
  {
    return findPluginIndex((std::string)pluginID);
  }
  MiniWindow *findWindow(const std::string_view windowName) const;
  WorldTab *tab() const;
};
