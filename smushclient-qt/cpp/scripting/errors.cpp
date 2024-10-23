#include "errors.h"
#include "luaapi.h"
#include "scriptapi.h"
extern "C"
{
#include "lualib.h"
}

static const char *errorHandlerKey = "trace";

// Private utils

inline bool checkError(int status)
{
  switch (status)
  {
  case LUA_OK:
    return false;
  case LUA_ERRMEM:
    throw std::bad_alloc();
  default:
    return true;
  }
}

// Private utils

QLatin1StringView getError(lua_State *L)
{
  size_t len;
  const char *message = lua_tolstring(L, -1, &len);
  return QLatin1StringView(message, len);
}

// Public functions

QString formatCompileError(lua_State *L)
{
  return ScriptApi::tr("Compile error: %1").arg(getError(L));
}

QString formatPanic(lua_State *L)
{
  return ScriptApi::tr("PANIC: unprotected error in call to Lua API: %1").arg(getError(L));
}

QString formatRuntimeError(lua_State *L)
{
  return ScriptApi::tr("Runtime error: %1").arg(getError(L));
}

bool api_pcall(lua_State *L, int nargs, int nreturn)
{
  if (checkError(lua_pcall(L, nargs, nreturn, 1))) [[unlikely]]
  {
    getApi(L).printError(formatRuntimeError(L));
    lua_pop(L, 1);
    return false;
  }

  return true;
}

int addErrorHandler(lua_State *L)
{
  lua_getglobal(L, LUA_DBLIBNAME);
  lua_getfield(L, -1, "traceback");
  lua_pushvalue(L, -1);
  lua_rawsetp(L, LUA_REGISTRYINDEX, errorHandlerKey);
  lua_remove(L, 1);
  return 1;
}

int pushErrorHandler(lua_State *L)
{
  lua_rawgetp(L, LUA_REGISTRYINDEX, errorHandlerKey);
  return 1;
}