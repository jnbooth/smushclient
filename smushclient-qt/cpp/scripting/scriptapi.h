#pragma once
#include "../bridge/variableview.h"
#include "../lookup.h"
#include "callback/filter.h"
#include "callback/key.h"
#include "databaseconnection.h"
#include "miniwindow/miniwindow.h"
#include "plugin.h"
#include "scriptenums.h"
#include "smushclient_qt/src/ffi/send_request.cxx.h"
#include <QtGui/QTextCursor>
#include <QtNetwork/QAbstractSocket>

#define SCRIPTING_VERSION "5.07"

class ImageFilter;
class MudScrollBar;
class MudBrowser;
class MudStatusBar;
class Notepads;
struct OutputLayout;
enum class SendTarget;
class SmushClient;
class Timekeeper;
class World;
class WorldTab;
struct lua_State;
template<typename T>
class TimerMap;

enum class SendFlag
{
  Echo = 1,
  Log = 2,
};
Q_DECLARE_FLAGS(SendFlags, SendFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SendFlags)

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  static int64_t GetUniqueNumber() noexcept;
  static QString MakeRegularExpression(std::string_view pattern) noexcept;
  static QStringList GetAlphaOptionList() noexcept;
  static QStringList GetOptionList() noexcept;
  static QVariant FontInfo(const QFont& font, int64_t infoType);
  static void SetClipboard(const QString& text);

  ScriptApi(const SmushClient& client,
            QAbstractSocket& socket,
            MudBrowser& output,
            Notepads& notepads,
            WorldTab& parent);
  ~ScriptApi() override;

  ApiCode AddAlias(size_t plugin,
                   std::string_view name,
                   std::string_view pattern,
                   std::string_view text,
                   QFlags<AliasFlag> flags,
                   std::string_view scriptName = std::string_view()) const;
  ApiCode AddTimer(size_t plugin,
                   std::string_view name,
                   int hour,
                   int minute,
                   double second,
                   std::string_view text,
                   QFlags<TimerFlag> flags,
                   std::string_view scriptName = std::string_view()) const;
  ApiCode AddTrigger(size_t plugin,
                     std::string_view name,
                     std::string_view pattern,
                     std::string_view text,
                     QFlags<TriggerFlag> flags,
                     const QColor& color,
                     std::string_view sound,
                     std::string_view scriptName,
                     SendTarget target,
                     int sequence = 100) const;
  int64_t BroadcastPlugin(size_t pluginIndex,
                          int64_t message,
                          std::string_view text) const;
  void ColourTell(const QColor& foreground,
                  const QColor& background,
                  const QString& text);
  int DatabaseClose(std::string_view databaseID);
  int DatabaseOpen(std::string_view databaseID,
                   std::string_view filename,
                   int flags);
  ApiCode DeleteAlias(size_t plugin, std::string_view name) const;
  size_t DeleteAliasGroup(size_t plugin, std::string_view group) const;
  ApiCode DeleteTimer(size_t plugin, std::string_view name) const;
  size_t DeleteTimerGroup(size_t plugin, std::string_view group) const;
  ApiCode DeleteTrigger(size_t plugin, std::string_view name) const;
  size_t DeleteTriggerGroup(size_t plugin, std::string_view group) const;
  ApiCode DeleteVariable(size_t plugin, std::string_view key) const;
  ApiCode DoAfter(size_t plugin,
                  double seconds,
                  const QString& text,
                  SendTarget target);
  ApiCode EnableAlias(size_t plugin,
                      std::string_view label,
                      bool enabled) const;
  ApiCode EnableAliasGroup(size_t plugin,
                           std::string_view group,
                           bool enabled) const;
  ApiCode EnablePlugin(std::string_view pluginID, bool enabled);
  ApiCode EnableTimer(size_t plugin,
                      std::string_view label,
                      bool enabled) const;
  ApiCode EnableTimerGroup(size_t plugin,
                           std::string_view group,
                           bool enabled) const;
  ApiCode EnableTrigger(size_t plugin,
                        std::string_view label,
                        bool enabled) const;
  ApiCode EnableTriggerGroup(size_t plugin,
                             std::string_view group,
                             bool enabled) const;
  QVariant GetAliasOption(size_t plugin,
                          std::string_view label,
                          std::string_view option) const;
  VariableView GetAlphaOption(size_t plugin, std::string_view name) const;
  QVariant GetCurrentValue(size_t pluginIndex, std::string_view option) const;
  QVariant GetInfo(int64_t infoType) const;
  QVariant GetLineInfo(int line, int64_t infoType) const;
  int GetLinesInBufferCount() const;
  int64_t GetOption(size_t plugin, std::string_view name) const;
  const std::string& GetPluginID(size_t pluginIndex) const;
  QVariant GetPluginInfo(std::string_view pluginID, int64_t infoType) const;
  QVariant GetStyleInfo(int line, int64_t style, int64_t infoType) const;
  QVariant GetTimerInfo(size_t pluginIndex,
                        std::string_view label,
                        int64_t infoType) const;
  QVariant GetTimerOption(size_t plugin,
                          std::string_view label,
                          std::string_view option) const;
  QVariant GetTriggerOption(size_t plugin,
                            std::string_view label,
                            std::string_view option) const;
  VariableView GetVariable(size_t pluginIndex, std::string_view key) const;
  VariableView GetVariable(std::string_view pluginID,
                           std::string_view key) const;
  void Hyperlink(const QString& action,
                 const QString& text,
                 const QString& hint,
                 const QColor& foreground,
                 const QColor& background,
                 bool url,
                 bool noUnderline);
  ApiCode IsAlias(size_t plugin, std::string_view label) const;
  ApiCode IsTimer(size_t plugin, std::string_view label) const;
  ApiCode IsTrigger(size_t plugin, std::string_view label) const;
  QColor PickColour(const QColor& hint) const;
  ApiCode PlaySound(size_t channel,
                    std::string_view path,
                    bool loop = false,
                    float volume = 1.0) const;
  ApiCode PlaySound(size_t channel,
                    const QString& path,
                    bool loop = false,
                    float volume = 1.0) const
  {
    const QByteArray utf8 = path.toUtf8();
    return PlaySound(
      channel, std::string_view(utf8.data(), utf8.size()), loop, volume);
  }
  ApiCode PlaySoundMemory(size_t channel,
                          QByteArrayView sound,
                          bool loop = false,
                          float volume = 1.0) const;
  ApiCode PluginSupports(std::string_view pluginID,
                         PluginCallbackKey routine) const;
  ApiCode Send(std::string_view text)
  {
    QByteArray bytes(text);
    return sendToWorld(bytes, SendFlag::Echo);
  }
  ApiCode Send(const QString& text)
  {
    QByteArray bytes = text.toUtf8();
    return sendToWorld(bytes, SendFlag::Echo);
  }
  ApiCode Send(QByteArray& bytes) { return sendToWorld(bytes, SendFlag::Echo); }
  ApiCode SendNoEcho(QByteArray& bytes)
  {
    return sendToWorld(bytes, SendFlags());
  }
  ApiCode SendPacket(QByteArrayView bytes);
  ApiCode SetAliasOption(size_t plugin,
                         std::string_view label,
                         std::string_view option,
                         std::string_view value) const;
  ApiCode SetAlphaOption(size_t plugin,
                         std::string_view name,
                         std::string_view value);
  ApiCode SetCursor(Qt::CursorShape cursor) const;
  ApiCode SetOption(size_t plugin, std::string_view name, int64_t value);
  void SetStatus(const QString& status) const;
  ApiCode SetTimerOption(size_t plugin,
                         std::string_view label,
                         std::string_view option,
                         std::string_view value) const;
  ApiCode SetTriggerOption(size_t plugin,
                           std::string_view label,
                           std::string_view option,
                           std::string_view value) const;
  bool SetVariable(size_t pluginIndex,
                   std::string_view key,
                   std::string_view value) const;
  void Simulate(std::string_view output) const;
  void StopEvaluatingTriggers() const;
  ApiCode StopSound(size_t channel = 0) const;
  void Tell(const QString& text);
  ApiCode TextRectangle(const QRect& rect,
                        int borderOffset,
                        const QColor& borderColor,
                        int borderWidth,
                        const QBrush& outsideFill);
  ApiCode TextRectangle(const OutputLayout& layout) const;
  ApiCode TextRectangle(const QMargins& margins,
                        int borderOffset,
                        const QColor& borderColor,
                        int borderWidth,
                        const QBrush& outsideFill) const;
  ApiCode TextRectangle() const;
  ApiCode WindowAddHotspot(size_t pluginIndex,
                           std::string_view windowName,
                           std::string_view hotspotID,
                           const QRect& geometry,
                           Hotspot::Callbacks&& callbacks,
                           const QString& tooltip,
                           Qt::CursorShape cursor,
                           Hotspot::Flags flags) const;
  ApiCode WindowButton(std::string_view windowName,
                       const QRect& rect,
                       MiniWindow::ButtonFrame frame,
                       MiniWindow::ButtonFlags flags) const;
  ApiCode WindowCreate(size_t pluginIndex,
                       std::string_view windowName,
                       const QPoint& location,
                       const QSize& size,
                       MiniWindow::Position position,
                       MiniWindow::Flags flags,
                       const QColor& fill);
  ApiCode WindowDeleteHotspot(std::string_view windowName,
                              std::string_view hotspotID) const;
  ApiCode WindowDrawImage(std::string_view windowName,
                          std::string_view imageID,
                          const QRectF& rect,
                          MiniWindow::DrawImageMode mode,
                          const QRectF& sourceRect) const;
  ApiCode WindowDrawImageAlpha(std::string_view windowName,
                               std::string_view imageID,
                               const QRectF& rect,
                               qreal opacity,
                               const QPointF& origin) const;
  ApiCode WindowEllipse(std::string_view windowName,
                        const QRectF& rect,
                        const QPen& pen,
                        const QBrush& brush = QBrush()) const;
  ApiCode WindowFilter(std::string_view windowName,
                       const ImageFilter& filter,
                       const QRect& rect = QRect()) const;
  ApiCode WindowFont(std::string_view windowName,
                     std::string_view fontID,
                     const QString& family,
                     qreal pointSize,
                     bool bold,
                     bool italic,
                     bool underline,
                     bool strikeout,
                     QFont::StyleHint hint) const;
  QVariant WindowFontInfo(std::string_view windowName,
                          std::string_view fontID,
                          int64_t infoType) const;
  ApiCode WindowFrame(std::string_view windowName,
                      const QRectF& rect,
                      const QColor& color1,
                      const QColor& color2) const;
  ApiCode WindowGradient(std::string_view windowName,
                         const QRectF& rect,
                         const QColor& color1,
                         const QColor& color2,
                         Qt::Orientation direction) const;
  QVariant WindowHotspotInfo(std::string_view windowName,
                             std::string_view hotspotID,
                             int64_t infoType) const;
  ApiCode WindowImageFromWindow(std::string_view windowName,
                                std::string_view imageID,
                                std::string_view sourceWindow) const;
  QVariant WindowInfo(std::string_view windowName, int64_t infoType) const;
  ApiCode WindowInvert(std::string_view windowName, const QRect& rect) const;
  ApiCode WindowLine(std::string_view windowName,
                     const QLineF& line,
                     const QPen& pen) const;
  ApiCode WindowLoadImage(std::string_view windowName,
                          std::string_view imageID,
                          const QString& filename) const;
  QVariant WindowMenu(std::string_view windowName,
                      const QPoint& location,
                      std::string_view menuString) const;
  ApiCode WindowMoveHotspot(std::string_view windowName,
                            std::string_view hotspotID,
                            const QRect& geometry) const;
  ApiCode WindowPolygon(std::string_view windowName,
                        const QPolygonF& polygon,
                        const QPen& pen,
                        const QBrush& brush,
                        bool close,
                        Qt::FillRule fillRule) const;
  ApiCode WindowPosition(std::string_view windowName,
                         const QPoint& location,
                         MiniWindow::Position position,
                         MiniWindow::Flags flags) const;
  ApiCode WindowRect(std::string_view windowName,
                     const QRectF& rect,
                     const QPen& pen,
                     const QBrush& brush = QBrush()) const;
  ApiCode WindowRoundedRect(std::string_view windowName,
                            const QRectF& rect,
                            qreal xRadius,
                            qreal yRadius,
                            const QPen& pen,
                            const QBrush& brush = QBrush()) const;
  ApiCode WindowResize(std::string_view windowName,
                       const QSize& size,
                       const QColor& fill) const;
  ApiCode WindowSetZOrder(std::string_view windowName, int64_t zOrder) const;
  ApiCode WindowShow(std::string_view windowName, bool show) const;
  qreal WindowText(std::string_view windowName,
                   std::string_view fontID,
                   std::string_view text,
                   const QRectF& rect,
                   const QColor& color,
                   bool unicode) const;
  int WindowTextWidth(std::string_view windowName,
                      std::string_view fontID,
                      std::string_view text,
                      bool unicode) const;
  ApiCode WindowUpdateHotspot(size_t pluginIndex,
                              std::string_view windowName,
                              std::string_view hotspotID,
                              Hotspot::CallbacksPartial&& callbacks) const;
  ApiCode WindowUnloadImage(std::string_view windowName,
                            std::string_view windowID) const;
  ApiCode WindowUnloadFont(std::string_view windowName,
                           std::string_view fontID) const;

  void appendHtml(const QString& html);
  void appendTell(const QString& text, const QTextCharFormat& format);
  void appendText(const QString& text, const QTextCharFormat& format);
  void appendText(const QString& text);
  void applyWorld(const World& world);
  void echo(const QString& text);
  void finishNote();
  const Plugin* getPlugin(std::string_view pluginID) const;
  constexpr Timekeeper& getTimekeeper() { return *timekeeper; }
  void handleSendRequest(const SendRequest& request);
  void initializePlugins();
  void moveCursor(QTextCursor::MoveOperation op, int count);
  void reinstallPlugin(size_t index);
  bool isPluginEnabled(size_t plugin) const
  {
    return !plugins[plugin].isDisabled();
  }
  ApiCode playFileRaw(const QString& path) const;
  void printError(const QString& message);
  void reloadWorldScript(const QString& worldScriptPath);
  void resetAllTimers();
  bool runScript(size_t plugin, std::string_view script) const
  {
    return plugins[plugin].runScript(script);
  }
  void sendCallback(PluginCallback& callback);
  bool sendCallback(PluginCallback& callback, size_t plugin);
  bool sendCallback(PluginCallback& callback, const QString& pluginID);
  void sendNaws();
  ApiCode sendToWorld(QByteArray& bytes, SendFlags flags)
  {
    return sendToWorld(bytes, QString::fromUtf8(bytes), flags);
  }
  ApiCode sendToWorld(const QString& text, SendFlags flags)
  {
    QByteArray bytes = text.toUtf8();
    return sendToWorld(bytes, text, flags);
  }
  void setNawsEnabled(bool enabled);
  void setOpen(bool open) const;
  ActionSource setSource(ActionSource source) noexcept;
  void setSuppressEcho(bool suppress) noexcept;
  void setWordUnderMenu(const QString& word) { wordUnderMenu = word; }
  void stackWindow(std::string_view windowName, MiniWindow& window) const;
  int startLine();
  constexpr MudStatusBar& statusBarWidgets() const noexcept
  {
    return *statusBar;
  }
  void updateTimestamp();

  constexpr std::vector<Plugin>::const_iterator cbegin() const noexcept
  {
    return plugins.cbegin();
  }
  constexpr std::vector<Plugin>::const_iterator begin() const noexcept
  {
    return plugins.begin();
  }
  constexpr std::vector<Plugin>::const_iterator cend() const noexcept
  {
    return plugins.cend();
  }
  constexpr std::vector<Plugin>::const_iterator end() const noexcept
  {
    return plugins.end();
  }

private:
  DatabaseConnection* findDatabase(std::string_view databaseID);
  size_t findPluginIndex(const std::string& pluginID) const;
  size_t findPluginIndex(std::string_view pluginID) const
  {
    return findPluginIndex(std::string(pluginID));
  }
  MiniWindow* findWindow(std::string_view windowName) const;
  bool finishQueuedSend(const SendRequest& request);
  void flushLine();
  void insertBlock();
  void insertText(const QString& text, const QTextCharFormat& format);
  ApiCode sendToWorld(QByteArray& bytes, const QString& text, SendFlags flags);

private:
  static constexpr size_t noSuchPlugin = std::numeric_limits<size_t>::max();

  ActionSource actionSource = ActionSource::Unknown;
  QRect assignedTextRectangle;
  CallbackFilter callbackFilter;
  const SmushClient& client;
  QTextCursor cursor;
  string_map<DatabaseConnection> databases;
  bool doNaws = false;
  bool doesNaws = false;
  QTextCharFormat echoFormat;
  bool echoInput = false;
  bool echoOnSameLine = false;
  QTextCharFormat errorFormat;
  bool hasLine = false;
  bool indentNext = false;
  QString indentText;
  QByteArray lastCommandSent;
  int lastTellPosition = -1;
  int lastLinePosition = -1;
  bool logNotes = false;
  QTextCharFormat noteFormat;
  Notepads& notepads;
  std::vector<Plugin> plugins;
  string_map<size_t> pluginIndices;
  MudScrollBar& scrollBar;
  TimerMap<SendRequest>* sendQueue;
  QAbstractSocket& socket;
  MudStatusBar* statusBar;
  bool suppressEcho = false;
  WorldTab& tab;
  Timekeeper* timekeeper;
  int64_t totalLinesSent = 0;
  int64_t totalPacketsSent = 0;
  QDateTime whenConnected;
  string_map<MiniWindow*> windows;
  QString wordUnderMenu;
  size_t worldScriptIndex = noSuchPlugin;
};
