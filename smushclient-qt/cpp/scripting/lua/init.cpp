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
#include <lauxlib.h>
#include <lualib.h>
  LUALIB_API int luaopen_bc(lua_State* L);
  LUALIB_API int luaopen_bit(lua_State* L);
  LUALIB_API int luaopen_cjson(lua_State* L);
  LUALIB_API int luaopen_lpeg(lua_State* L);
  LUALIB_API int luaopen_rex_pcre2(lua_State* L);
  LUALIB_API int luaopen_lsqlite3(lua_State* L);
}

using qlua::concatArgs;
using qlua::expectMaxArgs;
using qlua::getInteger;
using qlua::push;

namespace {
bool
getErrorDesc(lua_State* L, lua_Integer code)
{
  if (lua_getglobal(L, "error_desc") != LUA_TTABLE) {
    lua_pop(L, 1);
    return false;
  }
  push(L, code);
  if (lua_gettable(L, -2) != LUA_TSTRING) {
    lua_pop(L, 2);
    return false;
  }
  lua_remove(L, -2);
  return true;
}

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
L_check(lua_State* L)
{
  expectMaxArgs(L, 1);
  const lua_Integer result = getInteger(L, 1);
  if (result == 0) {
    return 0;
  }
  luaL_where(L, 1);
  if (!getErrorDesc(L, result)) {
    lua_pushfstring(L, "Unknown error code: %d", result);
  }
  lua_concat(L, 2);
  return lua_error(L);
}

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
  const QString output = concatArgs<QString>(L, 1, true);
#ifdef NDEBUG
  ScriptApi::of(L).Tell(output);
#endif
  qInfo() << "print(" << output << ")";
  return 0;
}
} // namespace

int
initLuaState(lua_State* L, size_t pluginIndex)
{
  lua_atpanic(L, &L_panic);
  lua_register(L, "check", L_check);
  lua_register(L, "print", L_print);
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
  luaopen_smushglobals(L);
  registerLuaWorld(L, pluginIndex);
  lua_settop(L, 0);
  addErrorHandler(L);
  return 1;
}
