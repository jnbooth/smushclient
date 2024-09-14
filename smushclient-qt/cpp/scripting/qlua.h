#pragma once
#include <QtCore/QString>
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
  std::string getString(lua_State *L, int idx);
  std::string getString(lua_State *L, int idx, std::string ifNil);
  const char *pushString(lua_State *L, const std::string &string);
  int loadQString(lua_State *L, const QString &chunk);
}
