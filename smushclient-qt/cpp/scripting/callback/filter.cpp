#include "filter.h"
#include "plugincallback.h"
extern "C"
{
#include "lua.h"
}

void
CallbackFilter::scan(lua_State* L)
{
  QByteArray emptyByteArray;
  const int top = lua_gettop(L);
  int i = 0;
  for (const NamedPluginCallback* callback : NamedPluginCallback::list()) {
    if ((++i & 7) == 0) {
      lua_settop(L, top);
    }
    if (lua_getglobal(L, callback->name()) == LUA_TFUNCTION) {
      filter |= callback->id();
    }
  }
  lua_settop(L, top);
}
