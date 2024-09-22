#pragma once
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <QtCore/QMargins>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QTextCursor>
#include "hotspot.h"
#include "miniwindow.h"
#include "plugin.h"
#include "scriptenums.h"

class ImageFilter;
class SmushClient;
class World;
class WorldTab;
struct lua_State;

class ScriptApi : public QObject
{
public:
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
  std::optional<std::string_view> GetVariable(size_t index, std::string_view key) const;
  std::optional<std::string_view> GetVariable(std::string_view pluginID, std::string_view key) const;
  const QString &GetPluginId(size_t index) const;
  QVariant GetPluginInfo(std::string_view pluginID, uint8_t infoType) const;
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
  ApiCode Send(const QByteArrayView &bytes);
  ApiCode SendNoEcho(const QByteArrayView &bytes) const;
  ApiCode SetOption(std::string_view name, const QVariant &variant) const;
  bool SetVariable(size_t index, std::string_view key, std::string_view value) const;
  void Tell(const QString &text);
  ApiCode TextRectangle(
      const QMargins &margins,
      int borderOffset,
      const QColor &borderColor,
      int borderWidth,
      const QBrush &outsideFill) const;
  ApiCode WindowAddHotspot(
      std::string_view pluginID,
      std::string_view windowName,
      std::string_view hotspotID,
      const QRect &geometry,
      Hotspot::Callbacks &&callbacks,
      const QString &tooltip,
      Qt::CursorShape cursor,
      bool trackHover) const;
  ApiCode WindowCreate(
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
  ApiCode WindowImageFromWindow(
      std::string_view windowName,
      std::string_view imageID,
      std::string_view sourceWindow) const;
  ApiCode WindowInvert(std::string_view windowName, const QRect &rect) const;
  ApiCode WindowLine(
      std::string_view windowName,
      const QLineF &line,
      const QPen &pen) const;
  ApiCode WindowLoadImage(
      std::string_view windowName,
      std::string_view imageID,
      const QString &filename) const;
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
