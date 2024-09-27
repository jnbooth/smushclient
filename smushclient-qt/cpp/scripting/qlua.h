#pragma once
#include <optional>
#include <string>
#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtGui/QPolygonF>
#include "miniwindow.h"
#include "scriptenums.h"
#include "cxx-qt-gen/ffi.cxxqt.h"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

namespace qlua
{
  inline constexpr QColor rgbCodeToColor(int rgb) noexcept
  {
    if (rgb == -1) [[unlikely]]
      return QColor();
    return QColor(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
  }

  inline int colorToRgbCode(const QColor &color)
  {
    if (!color.isValid()) [[unlikely]]
      return -1;
    int r, g, b;
    color.getRgb(&r, &g, &b);
    return b << 16 | g << 8 | r;
  }

  QString getError(lua_State *L);

  bool getBool(lua_State *L, int idx);
  bool getBool(lua_State *L, int idx, bool ifNil);

  QByteArrayView getBytes(lua_State *L, int idx);

  int getInt(lua_State *L, int idx);
  int getInt(lua_State *L, int idx, int ifNil);

  double getNumber(lua_State *L, int idx);
  double getNumber(lua_State *L, int idx, double ifNil);

  double getNumberOrBool(lua_State *L, int idx);
  double getNumberOrBool(lua_State *L, int idx, double ifNil);

  QColor getQColor(lua_State *L, int idx);
  QColor getQColor(lua_State *L, int idx, QColor ifNil);

  QString getQString(lua_State *L, int idx);
  QString getQString(lua_State *L, int idx, QString ifNil);

  QVariant getQVariant(lua_State *L, int idx, int type);
  inline QVariant getQVariant(lua_State *L, int idx) { return getQVariant(L, idx, lua_type(L, idx)); }

  std::string_view getString(lua_State *L, int idx);
  std::string_view getString(lua_State *L, int idx, std::string_view ifNil);

  QByteArray concatBytes(lua_State *L);
  QString concatStrings(lua_State *L, const QString &delimiter = QString());

  int loadQString(lua_State *L, const QString &chunk);

  const char *pushBytes(lua_State *L, const QByteArray &bytes);
  void pushQColor(lua_State *L, const QColor &color);
  void pushQHash(lua_State *L, const QVariantHash &variants);
  void pushQMap(lua_State *L, const QVariantMap &variants);
  const char *pushQString(lua_State *L, const QString &string);
  void pushQStrings(lua_State *L, const QStringList &strings);
  void pushQVariant(lua_State *L, const QVariant &variant);
  void pushQVariants(lua_State *L, const QVariantList &variants);
  const char *pushString(lua_State *L, std::string_view string);
  void pushStrings(lua_State *L, const std::vector<std::string> &strings);

  bool copyValue(lua_State *fromL, lua_State *toL, int idx);

  QLine getQLine(lua_State *L, int idxX1, int idxY1, int idxX2, int idxY2);
  QLineF getQLineF(lua_State *L, int idxX1, int idxY1, int idxX2, int idxY2);
  QMargins getQMargins(lua_State *L, int idxLeft, int idxTop, int idxRight, int idxBottom);
  QMarginsF getQMarginsF(lua_State *L, int idxLeft, int idxTop, int idxRight, int idxBottom);
  QPoint getQPoint(lua_State *L, int idxX, int idxY);
  QPointF getQPointF(lua_State *L, int idxX, int idxY);
  QSize getQSize(lua_State *L, int idxWidth, int idxHeight);
  QSizeF getQSizeF(lua_State *L, int idxWidth, int idxHeight);
  QRect getQRect(lua_State *L, int idxLeft, int idxTop, int idxWidth, int idxHeight);
  QRectF getQRectF(lua_State *L, int idxLeft, int idxTop, int idxWidth, int idxHeight);

  std::optional<Qt::BrushStyle> getBrush(
      lua_State *L,
      int idx,
      std::optional<Qt::BrushStyle> ifNil = std::nullopt);

  std::optional<CircleOp> getCircleOp(lua_State *L, int idx);

  std::optional<Qt::CursorShape> getCursor(
      lua_State *L,
      int idx,
      std::optional<Qt::CursorShape> ifNil = std::nullopt);

  std::optional<MiniWindow::DrawImageMode> getDrawImageMode(
      lua_State *L,
      int idx,
      std::optional<MiniWindow::DrawImageMode> ifNil = std::nullopt);

  std::optional<QFont::StyleHint> getFontHint(
      lua_State *L,
      int idx,
      std::optional<QFont::StyleHint> ifNil = std::nullopt);

  std::optional<QPen> getPen(lua_State *L, int idxColor, int idxStyle, int idxWidth);

  std::optional<QPolygonF> getQPolygonF(lua_State *L, int idx);

  std::optional<RectOp> getRectOp(lua_State *L, int idx);

  std::optional<SendTarget> getSendTarget(
      lua_State *L,
      int idx,
      std::optional<SendTarget> ifNil = std::nullopt);

  std::optional<MiniWindow::Position> getWindowPosition(
      lua_State *L,
      int idx,
      std::optional<MiniWindow::Position> ifNil = std::nullopt);
}
