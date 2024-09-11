#include "qlua.h"

extern "C"
{
#include "lauxlib.h"
}

bool checkIsSome(lua_State *L, int idx, int type, const char *name)
{
  int actualType = lua_type(L, idx);
  if (actualType <= 0)
    return false;
  luaL_argexpected(L, actualType == type, idx, name);
  return true;
}

lua_Integer toInt(lua_State *L, int idx)
{
  int isInt;
  int result = lua_tointegerx(L, idx, &isInt);
  luaL_argexpected(L, isInt, idx, "integer");
  return result;
}

QColor toColor(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  if (len == 0)
    return QColor();

  if (len == 0)
    return QColor();

  const QColor color = QColor::fromString(QAnyStringView(message, len));
  luaL_argcheck(L, color.isValid(), idx, "invalid color");
  return color;
}

QString toQString(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QString::fromUtf8(message, len);
}

bool qlua::getBool(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TBOOLEAN, idx, "boolean");
  return lua_toboolean(L, idx);
}

bool qlua::getBool(lua_State *L, int idx, bool ifNil)
{
  return checkIsSome(L, idx, LUA_TBOOLEAN, "boolean") ? lua_toboolean(L, idx) : ifNil;
}

lua_Integer qlua::getInt(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "integer");
  return toInt(L, idx);
}

lua_Integer qlua::getInt(lua_State *L, int idx, lua_Integer ifNil)
{
  return (checkIsSome(L, idx, LUA_TNUMBER, "integer")) ? toInt(L, idx) : ifNil;
}

lua_Number qlua::getNumber(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TNUMBER, idx, "number");
  return lua_tonumber(L, idx);
}

lua_Number qlua::getNumber(lua_State *L, int idx, lua_Number ifNil)
{
  return checkIsSome(L, idx, LUA_TNUMBER, "number") ? lua_tonumber(L, idx) : ifNil;
}

QByteArrayView qlua::borrowBytes(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QByteArrayView(message, len);
}

QColor qlua::getQColor(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toColor(L, idx);
}

QColor qlua::getQColor(lua_State *L, int idx, QColor ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toColor(L, idx) : ifNil;
}

QString qlua::getQString(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  return toQString(L, idx);
}

QString qlua::getQString(lua_State *L, int idx, QString ifNil)
{
  return checkIsSome(L, idx, LUA_TSTRING, "string") ? toQString(L, idx) : ifNil;
}

int qlua::loadQString(lua_State *L, const QString &chunk)
{
  const QByteArray utf8 = chunk.toUtf8();
  const char *data = utf8.constData();
  return luaL_loadbuffer(L, data, utf8.size(), data);
}
