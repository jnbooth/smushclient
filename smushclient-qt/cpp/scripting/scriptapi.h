#pragma once
#include "../mudcursor.h"
#include "../stringmap.h"
#include "../ui/notepad/notepad.h"
#include "callback/filter.h"
#include "callback/key.h"
#include "databaseconnection.h"
#include "miniwindow/miniwindow.h"
#include "plugin.h"
#include "scriptenums.h"
#include "smushclient_qt/src/ffi/send_request.cxx.h"
#include <QtCore/QPointer>
#include <QtCore/QUuid>
#include <QtGui/QTextCursor>
#include <QtNetwork/QAbstractSocket>
#include <QtWidgets/QLabel>

#define SCRIPTING_VERSION "5.07"

enum class ExportKind : uint8_t;
class ImageFilter;
class ImageWindow;
class MudScrollBar;
class MudBrowser;
class MudStatusBar;
class Notepads;
struct OutputLayout;
struct SendTimer;
class SmushClient;
class Timekeeper;
class World;
class WorldTab;
struct lua_State;
template<typename T, typename Handler>
class TimerMap;

namespace sendflag {
enum SendFlag
{
  Echo = 1,
  Log = 2,
  Immediate = 4,
};
} // namespace sendflag

using SendFlag = sendflag::SendFlag;
Q_DECLARE_FLAGS(SendFlags, SendFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SendFlags)

class ScriptApi : public QObject
{
  Q_OBJECT

public:
  static ApiCode AddFont(const QString& fileName);
  static QColor GetSysColor(SysColor sysColor);
  static int64_t GetUniqueNumber() noexcept;
  static QStringList GetAlphaOptionList() noexcept;
  static QStringList GetOptionList() noexcept;
  static QVariant FontInfo(const QFont& font, int64_t infoType);
  static QString MakeRegularExpression(std::string_view pattern) noexcept;
  static void SetClipboard(const QString& text);

  ScriptApi(SmushClient& client,
            QAbstractSocket& socket,
            MudBrowser& output,
            Notepads& notepads,
            WorldTab& parent);

  void ActivateClient() const;
  bool ActivateNotepad(const QString& name) const;
  ApiCode AddAlias(
    size_t plugin,
    std::string_view name,
    std::string_view pattern,
    std::string_view text,
    AliasFlags flags,
    std::string_view scriptName = std::string_view()) const noexcept;
  ApiCode AddTimer(
    size_t plugin,
    std::string_view name,
    int hour,
    int minute,
    double second,
    std::string_view text,
    TimerFlags flags,
    std::string_view scriptName = std::string_view()) const noexcept;
  ApiCode AddTrigger(size_t plugin,
                     std::string_view name,
                     std::string_view pattern,
                     std::string_view text,
                     TriggerFlags flags,
                     const QColor& color,
                     std::string_view sound,
                     std::string_view scriptName,
                     SendTarget target,
                     int sequence = 100) const noexcept;
  void AnsiNote(std::string_view text) const;
  void AppendToNotepad(const QString& name, const QString& text) const;
  int64_t BroadcastPlugin(size_t pluginIndex,
                          int64_t message,
                          std::string_view text) const;
  ApiCode CloseLog() const;
  bool CloseNotepad(const QString& name, bool querySave) const;
  void ColourTell(const QColor& foreground,
                  const QColor& background,
                  const QString& text);
  int DatabaseClose(std::string_view databaseID);
  int DatabaseOpen(std::string_view databaseID,
                   std::string_view filename,
                   int flags);
  ApiCode DeleteAlias(size_t plugin, std::string_view name) const noexcept;
  size_t DeleteAliasGroup(size_t plugin, std::string_view group) const noexcept;
  size_t DeleteTemporaryAliases() const noexcept;
  size_t DeleteTemporaryTimers() const noexcept;
  size_t DeleteTemporaryTriggers() const noexcept;
  ApiCode DeleteTimer(size_t plugin, std::string_view name) const noexcept;
  size_t DeleteTimerGroup(size_t plugin, std::string_view group) const noexcept;
  ApiCode DeleteTrigger(size_t plugin, std::string_view name) const noexcept;
  size_t DeleteTriggerGroup(size_t plugin,
                            std::string_view group) const noexcept;
  ApiCode DeleteVariable(size_t plugin, std::string_view key) const noexcept;
  ApiCode DoAfter(size_t plugin,
                  double seconds,
                  const QString& text,
                  SendTarget target);
  ApiCode EnableAlias(size_t plugin,
                      std::string_view label,
                      bool enabled) const noexcept;
  size_t EnableAliasGroup(size_t plugin,
                          std::string_view group,
                          bool enabled) const noexcept;
  ApiCode EnablePlugin(std::string_view pluginID, bool enabled);
  ApiCode EnableTimer(size_t plugin,
                      std::string_view label,
                      bool enabled) const noexcept;
  size_t EnableTimerGroup(size_t plugin,
                          std::string_view group,
                          bool enabled) const noexcept;
  ApiCode EnableTrigger(size_t plugin,
                        std::string_view label,
                        bool enabled) const noexcept;
  size_t EnableTriggerGroup(size_t plugin,
                            std::string_view group,
                            bool enabled) const noexcept;
  ApiCode Execute(const QString& command) const noexcept;
  QString ExportXML(size_t plugin,
                    ExportKind kind,
                    std::string_view name) const noexcept;
  ApiCode FlushLog() const;
  QVariant GetAliasOption(size_t plugin,
                          std::string_view label,
                          std::string_view option) const noexcept;
  std::string_view GetAlphaOption(size_t plugin,
                                  std::string_view name) const noexcept;
  QVariant GetCurrentValue(size_t pluginIndex,
                           std::string_view option) const noexcept;
  QVariant GetInfo(int64_t infoType) const;
  QVariant GetLineInfo(int line, int64_t infoType) const;
  int GetLinesInBufferCount() const;
  int GetNotepadLength(const QString& name) const;
  QStringList GetNotepadList(bool all = false) const;
  QString GetNotepadText(const QString& name) const;
  QRect GetNotepadWindowPosition(const QString& name) const;
  int64_t GetOption(size_t plugin, std::string_view name) const noexcept;
  const std::string& GetPluginID(size_t pluginIndex) const;
  QVariant GetPluginInfo(std::string_view pluginID,
                         int64_t infoType) const noexcept;
  QVariant GetStyleInfo(int line, int64_t style, int64_t infoType) const;
  QVariant GetTimerInfo(size_t pluginIndex,
                        std::string_view label,
                        int64_t infoType) const;
  QVariant GetTimerOption(size_t plugin,
                          std::string_view label,
                          std::string_view option) const noexcept;
  QVariant GetTriggerOption(size_t plugin,
                            std::string_view label,
                            std::string_view option) const noexcept;
  std::string_view GetVariable(size_t pluginIndex,
                               std::string_view key) const noexcept;
  std::string_view GetVariable(std::string_view pluginID,
                               std::string_view key) const noexcept;
  void Hyperlink(const QString& action,
                 const QString& text,
                 const QString& hint,
                 const QColor& foreground,
                 const QColor& background,
                 bool url,
                 bool noUnderline);
  ApiCode IsAlias(size_t plugin, std::string_view label) const noexcept;
  bool IsLogOpen() const noexcept;
  ApiCode IsTimer(size_t plugin, std::string_view label) const noexcept;
  ApiCode IsTrigger(size_t plugin, std::string_view label) const noexcept;
  ApiCode LogSend(QByteArray& bytes);
  ApiCode OpenLog(std::string_view logFileName, bool append) const;
  bool NotepadColour(const QString& name,
                     const QColor& foreground,
                     const QColor& background) const;
  bool NotepadFont(const QString& name, const QTextCharFormat& format) const;
  bool NotepadReadOnly(const QString& name, bool readOnly) const;
  bool NotepadSaveMethod(const QString& name, Notepad::SaveMethod method) const;
  QColor PickColour(const QColor& hint) const;
  ApiCode PlaySound(size_t channel,
                    std::string_view path,
                    bool loop = false,
                    float volume = 1.0) const noexcept;
  ApiCode PlaySound(size_t channel,
                    const QString& path,
                    bool loop = false,
                    float volume = 1.0) const noexcept;
  ApiCode PlaySoundMemory(size_t channel,
                          QByteArrayView sound,
                          bool loop = false,
                          float volume = 1.0) const noexcept;
  ApiCode PluginSupports(std::string_view pluginID,
                         PluginCallbackKey routine) const;
  bool ReplaceNotepad(const QString& name, const QString& text) const;
  bool SaveNotepad(const QString& name,
                   const QString& filePath,
                   bool replace) const;
  ApiCode Send(std::string_view text);
  ApiCode Send(const QString& text);
  ApiCode Send(QByteArray& bytes);
  ApiCode SendImmediate(QByteArray& bytes);
  ApiCode SendNoEcho(QByteArray& bytes);
  ApiCode SendPacket(QByteArrayView bytes);
  ApiCode SendPush(QByteArray& bytes);
  bool SendToNotepad(const QString& name, const QString& text) const;
  ApiCode SetAliasOption(size_t plugin,
                         std::string_view label,
                         std::string_view option,
                         std::string_view value) const noexcept;
  ApiCode SetAlphaOption(size_t plugin,
                         std::string_view name,
                         std::string_view value);
  QColor SetBackgroundColour(const QColor& color) const;
  ApiCode SetBackgroundImage(const QString& path,
                             MiniWindow::Position position);
  ApiCode SetCursor(Qt::CursorShape cursor) const;
  QColor SetForegroundColour(const QColor& color) const;
  ApiCode SetForegroundImage(const QString& path,
                             MiniWindow::Position position);
  QColor SetHighlightColour(const QColor& color) const;
  void SetMainTitle(const QString& title) const;
  ApiCode SetOption(size_t plugin, std::string_view name, int64_t value);
  void SetStatus(const QString& status) const;
  ApiCode SetTimerOption(size_t plugin,
                         std::string_view label,
                         std::string_view option,
                         std::string_view value) const noexcept;
  void SetTitle(const QString& title) const;
  ApiCode SetTriggerOption(size_t plugin,
                           std::string_view label,
                           std::string_view option,
                           std::string_view value) const noexcept;
  bool SetVariable(size_t pluginIndex,
                   std::string_view key,
                   std::string_view value) const noexcept;
  void Simulate(std::string_view output) const noexcept;
  void StopEvaluatingTriggers() const noexcept;
  ApiCode StopSound(size_t channel = 0) const noexcept;
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
  ApiCode WindowArc(std::string_view windowName,
                    const QRectF& rect,
                    const QPointF& start,
                    const QPointF& end,
                    const QPen& pen) const;
  ApiCode WindowBlendImage(std::string_view windowName,
                           std::string_view imageID,
                           const QRectF& rect,
                           BlendMode mode,
                           qreal opacity,
                           const QRectF& sourceRect) const;
  ApiCode WindowButton(std::string_view windowName,
                       const QRect& rect,
                       ButtonFrame frame,
                       MiniWindow::ButtonFlags flags) const;
  ApiCode WindowCreate(size_t pluginIndex,
                       std::string_view windowName,
                       const QPoint& location,
                       const QSize& size,
                       MiniWindow::Position position,
                       MiniWindow::Flags flags,
                       const QColor& fill);
  ApiCode WindowCreateImage(std::string_view windowName,
                            std::string_view imageID,
                            std::array<int64_t, 8> rows) const;
  ApiCode WindowDelete(std::string_view windowName) const;
  ApiCode WindowDeleteAllHotspots(std::string_view windowName) const;
  ApiCode WindowDeleteHotspot(std::string_view windowName,
                              std::string_view hotspotID) const;
  ApiCode WindowDrawImage(std::string_view windowName,
                          std::string_view imageID,
                          const QRectF& rect,
                          DrawImageMode mode,
                          const QRectF& sourceRect) const;
  ApiCode WindowDrawImageAlpha(std::string_view windowName,
                               std::string_view imageID,
                               const QRectF& rect,
                               qreal opacity,
                               const QPointF& origin) const;
  ApiCode WindowEllipse(std::string_view windowName,
                        const QRectF& rect,
                        const QPen& pen,
                        const QBrush& brush) const;
  ApiCode WindowEllipse(std::string_view windowName,
                        const QRectF& rect,
                        const QPen& pen,
                        const QColor& brushColor,
                        std::string_view imageID) const;
  ApiCode WindowFilter(std::string_view windowName,
                       const ImageFilter& filter,
                       const QRect& rect) const;
  ApiCode WindowFont(std::string_view windowName,
                     std::string_view fontID,
                     const QString& family,
                     qreal pointSize,
                     bool bold,
                     bool italic,
                     bool underline,
                     bool strikeout,
                     QFont::StyleHint hint) const;
  std::vector<std::string_view> WindowFontList(
    std::string_view windowName) const;
  QVariant WindowFontInfo(std::string_view windowName,
                          std::string_view fontID,
                          int64_t infoType) const;
  ApiCode WindowFrame(std::string_view windowName,
                      const QRectF& rect,
                      const QColor& color1,
                      const QColor& color2) const;
  ApiCode WindowGetImageAlpha(std::string_view windowName,
                              std::string_view imageID,
                              const QRectF& rect,
                              const QPointF& point) const;
  std::optional<QColor> WindowGetPixel(std::string_view windowName,
                                       const QPoint& point) const;
  ApiCode WindowGradient(std::string_view windowName,
                         const QRectF& rect,
                         const QColor& color1,
                         const QColor& color2,
                         Qt::Orientation direction) const;
  QVariant WindowHotspotInfo(std::string_view windowName,
                             std::string_view hotspotID,
                             int64_t infoType) const;
  std::vector<std::string_view> WindowHotspotList(
    std::string_view windowName) const;
  ApiCode WindowHotspotTooltip(std::string_view windowName,
                               std::string_view hotspotID,
                               const QString& tooltip) const;
  ApiCode WindowImageFromWindow(std::string_view windowName,
                                std::string_view imageID,
                                std::string_view sourceWindow) const;
  QVariant WindowImageInfo(std::string_view windowName,
                           std::string_view imageID,
                           int64_t infoType) const;
  std::vector<std::string_view> WindowImageList(
    std::string_view windowName) const;
  QVariant WindowInfo(std::string_view windowName, int64_t infoType) const;
  ApiCode WindowInvert(std::string_view windowName, const QRect& rect) const;
  ApiCode WindowLine(std::string_view windowName,
                     const QLineF& line,
                     const QPen& pen) const;
  std::vector<std::string_view> WindowList() const noexcept;
  ApiCode WindowLoadImage(std::string_view windowName,
                          std::string_view imageID,
                          const QString& filename) const;
  QVariant WindowMenu(std::string_view windowName,
                      const QPoint& location,
                      std::string_view menuString) const;
  ApiCode WindowMergeImageAlpha(std::string_view windowName,
                                std::string_view imageID,
                                std::string_view maskID,
                                const QRect& targetRect,
                                MergeMode mode,
                                qreal opacity,
                                const QRect& sourceRect) const;
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
                     const QBrush& brush) const;
  ApiCode WindowRect(std::string_view windowName,
                     const QRectF& rect,
                     const QPen& pen,
                     const QColor& brushColor,
                     std::string_view imageID) const;
  ApiCode WindowRoundedRect(std::string_view windowName,
                            const QRectF& rect,
                            qreal xRadius,
                            qreal yRadius,
                            const QPen& pen,
                            const QBrush& brush) const;
  ApiCode WindowRoundedRect(std::string_view windowName,
                            const QRectF& rect,
                            qreal xRadius,
                            qreal yRadius,
                            const QPen& pen,
                            const QColor& brushColor,
                            std::string_view imageID) const;
  ApiCode WindowResize(std::string_view windowName,
                       const QSize& size,
                       const QColor& fill) const;
  ApiCode WindowSetPixel(std::string_view windowName,
                         const QPoint& point,
                         const QColor& color) const;
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
  ApiCode WindowTransformImage(std::string_view windowName,
                               std::string_view imageID,
                               MergeMode mode,
                               const QTransform& transform) const;
  ApiCode WindowUnloadFont(std::string_view windowName,
                           std::string_view fontID) const;
  ApiCode WindowUnloadImage(std::string_view windowName,
                            std::string_view windowID) const;
  ApiCode WindowUpdateHotspot(size_t pluginIndex,
                              std::string_view windowName,
                              std::string_view hotspotID,
                              Hotspot::CallbacksPartial&& callbacks) const;
  ApiCode WindowWrite(std::string_view windowName,
                      const QString& filename) const;
  ApiCode WriteLog(std::string_view message) const;

  void applyWorld(const World& world);
  void finishNote();
  const Plugin* getPlugin(std::string_view pluginID) const noexcept;
  Timekeeper& getTimekeeper() { return *timekeeper; }
  void handleSendRequest(const SendRequest& request);
  bool isPluginEnabled(size_t plugin) const noexcept
  {
    return !plugins[plugin].isDisabled();
  }
  ApiCode playFileRaw(std::string_view path) const noexcept;
  void printError(const QString& message);
  void reloadWorldScript(const QString& worldScriptPath);
  void resetAllTimers();
  bool runScript(size_t plugin, std::string_view script, const char* name) const
  {
    return plugins[plugin].runScript(script, name);
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
  ApiCode sendToWorld(QByteArray& bytes, const QString& text, SendFlags flags);
  void setNawsEnabled(bool enabled) noexcept;
  void setOpen(bool open) noexcept;
  void setPluginEnabled(size_t plugin, bool enable = true);
  ActionSource setSource(ActionSource source) noexcept;
  void setWordUnderMenu(const QString& word) noexcept { wordUnderMenu = word; }
  void stackWindow(std::string_view windowName, MiniWindow& window) const;
  MudStatusBar* statusBar() const noexcept { return statusBarPtr.get(); }

  std::vector<Plugin>::const_iterator cbegin() const noexcept
  {
    return plugins.cbegin();
  }
  std::vector<Plugin>::const_iterator begin() const noexcept
  {
    return plugins.begin();
  }
  std::vector<Plugin>::const_iterator cend() const noexcept
  {
    return plugins.cend();
  }
  std::vector<Plugin>::const_iterator end() const noexcept
  {
    return plugins.end();
  }

public slots:
  void initializePlugins();
  void onResize(bool finished);
  void onTimerSent(const SendTimer& timer);
  void reinstallPlugin(size_t index);

private:
  static void activateWindow(QWidget* widget);
  DatabaseConnection* findDatabase(std::string_view databaseID) noexcept;
  size_t findPluginIndex(std::string_view pluginID) const noexcept;
  MiniWindow* findWindow(std::string_view windowName) const noexcept;
  bool finishQueuedSend(const SendRequest& request);
  ApiCode setImage(const QString& path,
                   MiniWindow::Position position,
                   bool above);

private:
  static constexpr size_t noSuchPlugin = 0xFFFFFFFFFFFFFFFF;

  ActionSource actionSource = ActionSource::Unknown;
  QRect assignedTextRectangle;
  ImageWindow* backgroundImage = nullptr;
  CallbackFilter callbackFilter;
  const SmushClient& client;
  bool closed = true;
  QPointer<MudCursor> cursor;
  string_map<DatabaseConnection> databases;
  bool doNaws = false;
  bool doesNaws = false;
  bool echoInput = false;
  ImageWindow* foregroundImage = nullptr;
  QByteArray lastCommandSent;
  Notepads& notepads;
  std::vector<Plugin> plugins;
  string_map<size_t> pluginIndices;
  MudScrollBar& scrollBar;
  TimerMap<SendRequest, ScriptApi>* sendQueue;
  QAbstractSocket& socket;
  std::unique_ptr<MudStatusBar> statusBarPtr;
  WorldTab& tab;
  Timekeeper* timekeeper;
  int64_t totalLinesSent = 0;
  int64_t totalPacketsSent = 0;
  QDateTime whenConnected;
  string_map<std::unique_ptr<MiniWindow>> windows;
  QString wordUnderMenu;
  QUuid worldID;
  size_t worldScriptIndex = noSuchPlugin;
};
