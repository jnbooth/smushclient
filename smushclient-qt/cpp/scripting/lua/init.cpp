#include "init.h"
#include "../qlua.h"
#include "api.h"
#include "errors.h"
#include "globals.h"
#ifdef NDEBUG
#include "../scriptapi.h"
#endif
#include "utils.h"
#include <QtCore/QString>
#include <QtWidgets/QErrorMessage>
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
  LUALIB_API int luaopen_bc(lua_State* L);
  LUALIB_API int luaopen_bit(lua_State* L);
  LUALIB_API int luaopen_cjson(lua_State* L);
  LUALIB_API int luaopen_lpeg(lua_State* L);
  LUALIB_API int luaopen_rex_pcre2(lua_State* L);
  LUALIB_API int luaopen_lsqlite3(lua_State* L);
}

namespace {
void
setlib(lua_State* L, const char* name)
{
  lua_pushvalue(L, -1);
  lua_setglobal(L, name);
  lua_setfield(L, 1, name);
}
} // namespace

namespace {
int
L_panic(lua_State* L)
{
  const QString message = formatPanic(L);
  qCritical() << "panic(" << message << ")";
  QErrorMessage::qtHandler()->showMessage(message);
  return 0;
}

int
L_print(lua_State* L)
{
  const QString output = qlua::concatStrings(L);
#ifdef NDEBUG
  getApi(L)->Tell(output);
#endif
  qInfo() << "print(" << output << ")";
  return 0;
}
} // namespace

int
initLuaState(lua_State* L)
{
  lua_atpanic(L, &L_panic);
  lua_pushcfunction(L, L_print);
  lua_setglobal(L, "print");
  luaL_openlibs(L);
  lua_settop(L, 0);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaopen_bc(L);
  setlib(L, "bc");
  luaopen_bit(L);
  setlib(L, "bit");
  luaopen_cjson(L);
  setlib(L, "cjson");
  luaopen_lpeg(L);
  setlib(L, "lpeg");
  luaopen_rex_pcre2(L);
  setlib(L, "rex");
  luaopen_lsqlite3(L);
  setlib(L, "sqlite3");
  luaopen_utils(L);
  setlib(L, "utils");
  registerLuaGlobals(L);
  registerLuaWorld(L);
  lua_settop(L, 0);
  addErrorHandler(L);
  return 1;
}
