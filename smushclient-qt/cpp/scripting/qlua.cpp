#include "qlua.h"

extern "C"
{
#include "lauxlib.h"
}

QColor qlua::getQColor(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  if (len == 0)
    return QColor();

  const QColor color = QColor::fromString(QAnyStringView(message, len));
  luaL_argcheck(L, color.isValid(), idx, "invalid color");
  return color;
}

QString qlua::getQString(lua_State *L, int idx)
{
  luaL_argexpected(L, lua_type(L, idx) == LUA_TSTRING, idx, "string");
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return QString::fromUtf8(message, len);
}

int qlua::loadQString(lua_State *L, const QString &chunk)
{
  const QByteArray utf8 = chunk.toUtf8();
  const char *data = utf8.constData();
  return luaL_loadbuffer(L, data, utf8.size(), data);
}
