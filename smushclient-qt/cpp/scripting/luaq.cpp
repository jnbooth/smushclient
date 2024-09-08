#include "luaq.h"

QString luaQ_toqstring(lua_State *L, int idx)
{
  size_t len;
  const char *message = lua_tolstring(L, idx, &len);
  return len == 0 ? QString() : QString::fromUtf8(QByteArrayView(message, len));
}

int luaQ_loadqstring(lua_State *L, const QString &chunk)
{
  QByteArray utf8 = chunk.toUtf8();
  const char *data = utf8.constData();
  return luaL_loadbuffer(L, data, utf8.size(), data);
}
