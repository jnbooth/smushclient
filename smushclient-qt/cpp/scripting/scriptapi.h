#pragma once
#include <optional>
#include <string>
#include <vector>
#include <QtCore/QMargins>
#include <QtCore/QUuid>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtNetwork/QAbstractSocket>
#include <QtGui/QTextCursor>
#include "databaseconnection.h"
#include "hotspot.h"
#include "miniwindow.h"
#include "plugin.h"
#include "scriptenums.h"
#include "../lookup.h"
#include "../variableview.h"

#define SCRIPTING_VERSION "5.07"

constexpr size_t noSuchPlugin = std::numeric_limits<size_t>::max();

class ImageFilter;
class MudScrollBar;
class MudStatusBar;
class Notepads;
struct OutputLayout;
enum class SendTarget;
class SmushClient;
class Timekeeper;
enum class TriggerBool : uint8_t;
class World;
class WorldTab;
struct lua_State;
template <typename T>
class TimerMap;

struct QueuedSend
{
  size_t plugin;
  SendTarget target;
  QString text;
};

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  static int GetUniqueNumber() noexcept;
  static QString MakeRegularExpression(const QString &pattern) noexcept;
  static void SetClipboard(const QString &text);

  ScriptApi(MudStatusBar *statusBar, Notepads *notepads, WorldTab *parent);
  ~ScriptApi();

  ApiCode AddAlias(
      size_t plugin,
      const QString &name,
      const QString &pattern,
      const QString &text,
      QFlags<AliasFlag> flags,
      const QString &scriptName = QString()) const;
  ApiCode AddTimer(
      size_t plugin,
      const QString &name,
      int hour,
      int minute,
      double second,
      const QString &text,
      QFlags<TimerFlag> flags,
      const QString &scriptName = QString()) const;
  ApiCode AddTrigger(
      size_t plugin,
      const QString &name,
      const QString &pattern,
      const QString &text,
      QFlags<TriggerFlag> flags,
      const QColor &color,
      const QString &sound,
      const QString &scriptName,
      SendTarget target,
      int sequence = 100) const;
  int BroadcastPlugin(size_t pluginIndex, int message, std::string_view text) const;
  void ColourTell(const QColor &foreground, const QColor &background, const QString &text);
  int DatabaseClose(std::string_view databaseID);
  int DatabaseOpen(std::string_view databaseID, std::string_view filename, int flags);
  ApiCode DeleteAlias(size_t plugin, const QString &name) const;
  size_t DeleteAliases(size_t plugin, const QString &group) const;
  ApiCode DeleteTimer(size_t plugin, const QString &name) const;
  size_t DeleteTimers(size_t plugin, const QString &group) const;
  ApiCode DeleteTrigger(size_t plugin, const QString &name) const;
  size_t DeleteTriggers(size_t plugin, const QString &group) const;
  ApiCode DeleteVariable(size_t plugin, std::string_view key) const;
  ApiCode DoAfter(size_t plugin, double seconds, const QString &text, SendTarget target);
  ApiCode EnableAlias(size_t plugin, const QString &label, bool enabled) const;
  ApiCode EnableAliasGroup(size_t plugin, const QString &group, bool enabled) const;
  ApiCode EnablePlugin(std::string_view pluginID, bool enabled);
  ApiCode EnableTimer(size_t plugin, const QString &label, bool enabled) const;
  ApiCode EnableTimerGroup(size_t plugin, const QString &group, bool enabled) const;
  ApiCode EnableTrigger(size_t plugin, const QString &label, bool enabled) const;
  ApiCode EnableTriggerGroup(size_t plugin, const QString &group, bool enabled) const;
  QVariant FontInfo(const QFont &font, int infoType) const;
  QVariant GetInfo(int infoType) const;
  QVariant GetLineInfo(int line, int infoType) const;
  int GetLinesInBufferCount() const;
  QVariant GetOption(std::string_view name) const;
  const std::string &GetPluginID(size_t pluginIndex) const;
  QVariant GetPluginInfo(std::string_view pluginID, int infoType) const;
  QVariant GetStyleInfo(int line, int style, int infoType) const;
  QVariant GetTimerInfo(size_t pluginIndex, const QString &label, int infoType) const;
  VariableView GetVariable(size_t pluginIndex, std::string_view key) const;
  VariableView GetVariable(std::string_view pluginID, std::string_view key) const;
  void Hyperlink(
      const QString &action,
      const QString &text,
      const QString &hint,
      const QColor &foreground,
      const QColor &background,
      bool url,
      bool noUnderline);
  ApiCode IsAlias(size_t plugin, const QString &label) const;
  ApiCode IsTimer(size_t plugin, const QString &label) const;
  ApiCode IsTrigger(size_t plugin, const QString &label) const;
  QColor PickColour(const QColor &hint) const;
  ApiCode PlaySound(size_t channel, const QString &path, bool loop = false, float volume = 1.0);
  ApiCode PlaySoundMemory(size_t channel, QByteArrayView sound, bool loop = false, float volume = 1.0);
  ApiCode PluginSupports(std::string_view pluginID, PluginCallbackKey routine) const;
  ApiCode Send(QByteArray &bytes);
  ApiCode Send(const QString &text);
  ApiCode SendNoEcho(QByteArray &bytes);
  ApiCode SendPacket(QByteArrayView bytes) const;
  ApiCode SetCursor(Qt::CursorShape cursor) const;
  ApiCode SetOption(std::string_view name, const QVariant &variant) const;
  void SetStatus(const QString &status) const;
  ApiCode SetTriggerGroup(size_t plugin, const QString &label, const QString &group) const;
  ApiCode SetTriggerOption(size_t plugin, const QString &label, TriggerBool option, bool value) const;
  bool SetVariable(size_t pluginIndex, std::string_view key, std::string_view value) const;
  void StopEvaluatingTriggers() const;
  ApiCode StopSound(size_t channel = 0);
  void Tell(const QString &text);
  ApiCode TextRectangle(
      const QRect &rect,
      int borderOffset,
      const QColor &borderColor,
      int borderWidth,
      const QBrush &outsideFill) const;
  ApiCode TextRectangle(const OutputLayout &layout) const;
  ApiCode TextRectangle(
      const QMargins &margins,
      int borderOffset,
      const QColor &borderColor,
      int borderWidth,
      const QBrush &outsideFill) const;
  ApiCode TextRectangle() const;
  ApiCode WindowAddHotspot(
      size_t pluginIndex,
      std::string_view windowName,
      std::string_view hotspotID,
      const QRect &geometry,
      Hotspot::Callbacks &&callbacks,
      const QString &tooltip,
      Qt::CursorShape cursor,
      Hotspot::Flags flags) const;
  ApiCode WindowButton(
      std::string_view windowName,
      const QRect &rect,
      MiniWindow::ButtonFrame frame,
      MiniWindow::ButtonFlags flags) const;
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
      const QString &family,
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

  void appendHtml(const QString &html);
  void appendTell(const QString &text, const QTextCharFormat &format);
  void appendText(const QString &text, const QTextCharFormat &format);
  void appendText(const QString &text);
  void applyWorld(const World &world);
  SmushClient *client() const;
  void echo(const QString &text);
  void finishNote();
  const Plugin *getPlugin(std::string_view pluginID) const;
  void initializePlugins();
  void reinstallPlugin(size_t index);
  inline bool isPluginEnabled(size_t plugin) const { return !plugins[plugin].disabled(); }
  void printError(const QString &message);
  void reloadWorldScript(const QString &worldScriptPath);
  void resetAllTimers();
  inline bool runScript(size_t plugin, const QString &script) const
  {
    return plugins[plugin].runScript(script);
  }
  bool runScript(const QString &pluginID, const QString &script) const;
  void sendCallback(PluginCallback &callback);
  bool sendCallback(PluginCallback &callback, size_t plugin);
  bool sendCallback(PluginCallback &callback, const QString &pluginID);
  void sendNaws() const;
  void sendTo(
      size_t plugin,
      SendTarget target,
      const QString &text,
      const QString &destination = QString());
  void setNawsEnabled(bool enabled);
  void setOpen(bool open) const;
  ActionSource setSource(ActionSource source) noexcept;
  void setSuppressEcho(bool suppress) noexcept;
  void stackWindow(std::string_view windowName, MiniWindow *window) const;
  int startLine();
  constexpr MudStatusBar *statusBarWidgets() const
  {
    return statusBar;
  }
  void updateTimestamp();

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

public:
  Timekeeper *timekeeper;

private:
  DatabaseConnection *findDatabase(const std::string_view databaseID);
  size_t findPluginIndex(const std::string &pluginID) const;
  inline size_t findPluginIndex(const std::string_view pluginID) const
  {
    return findPluginIndex((std::string)pluginID);
  }
  MiniWindow *findWindow(const std::string_view windowName) const;
  bool finishQueuedSend(const QueuedSend &send);
  void flushLine();
  WorldTab *tab() const;

private:
  ActionSource actionSource = ActionSource::Unknown;
  CallbackFilter callbackFilter{};
  QTextCursor cursor;
  string_map<DatabaseConnection> databases{};
  bool doNaws = false;
  bool doesNaws = false;
  QTextCharFormat echoFormat{};
  bool echoOnSameLine = false;
  QTextCharFormat errorFormat{};
  bool hasLine = false;
  bool indentNext = false;
  QString indentText{};
  int lastTellPosition = -1;
  QTextCharFormat noteFormat{};
  Notepads *notepads;
  std::vector<Plugin> plugins{};
  string_map<size_t> pluginIndices{};
  MudScrollBar *scrollBar;
  TimerMap<QueuedSend> *sendQueue;
  QAbstractSocket *socket;
  MudStatusBar *statusBar;
  bool suppressEcho = false;
  QDateTime whenConnected;
  string_map<MiniWindow *> windows{};
  size_t worldScriptIndex = noSuchPlugin;
};
