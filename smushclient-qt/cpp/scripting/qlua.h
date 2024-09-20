#pragma once
#include <optional>
#include <string>
#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include "miniwindow.h"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

namespace qlua
{
  inline constexpr QColor rgbCodeToColor(int rgb) noexcept
  {
    return QColor(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
  }

  inline int colorToRgbCode(const QColor &color)
  {
    if (!color.isValid())
      return -1;
    int r, g, b;
    color.getRgb(&r, &g, &b);
    return b << 16 | g << 8 | r;
  }

  QString getError(lua_State *L);

  QByteArrayView borrowBytes(lua_State *L, int idx);

  bool getBool(lua_State *L, int idx);
  bool getBool(lua_State *L, int idx, bool ifNil);

  lua_Integer getInt(lua_State *L, int idx);
  lua_Integer getInt(lua_State *L, int idx, lua_Integer ifNil);

  lua_Number getNumber(lua_State *L, int idx);
  lua_Number getNumber(lua_State *L, int idx, lua_Number ifNil);

  QColor getQColor(lua_State *L, int idx);
  QColor getQColor(lua_State *L, int idx, QColor ifNil);

  QString getQString(lua_State *L, int idx);
  QString getQString(lua_State *L, int idx, QString ifNil);

  QVariant getQVariant(lua_State *L, int idx, int type);
  inline QVariant getQVariant(lua_State *L, int idx) { return getQVariant(L, idx, lua_type(L, idx)); }

  std::string_view getString(lua_State *L, int idx);
  std::string_view getString(lua_State *L, int idx, std::string_view ifNil);

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

  std::optional<Qt::BrushStyle> getBrush(
      lua_State *L,
      int idx,
      std::optional<Qt::BrushStyle> ifNil = nullopt);

  std::optional<Qt::CursorShape> getCursor(
      lua_State *L,
      int idx,
      std::optional<Qt::CursorShape> ifNil = nullopt);

  std::optional<QFont::StyleHint> getFontHint(
      lua_State *L,
      int idx,
      std::optional<QFont::StyleHint> ifNil = nullopt);

  std::optional<QPen> getPen(lua_State *L, int idxColor, int idxStyle, int idxWidth);

  std::optional<MiniWindow::Position> getWindowPosition(
      lua_State *L,
      int idx,
      std::optional<MiniWindow::Position> ifNil = nullopt);

}
