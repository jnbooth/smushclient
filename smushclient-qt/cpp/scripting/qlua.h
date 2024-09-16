#pragma once
#include <QtCore/QVariant>
#include <QtGui/QColor>

extern "C"
{
#include "lua.h"
}

namespace qlua
{
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
  std::string_view getString(lua_State *L, int idx, std::string ifNil);

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
}
