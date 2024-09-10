#include "lualibs.h"
extern "C"
{
#include "lualib.h"
}

void openLuaLibs(lua_State *L)
{
  luaL_openlibs(L);
}
