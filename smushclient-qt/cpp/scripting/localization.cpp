#include "localization.h"
#include "scriptapi.h"
extern "C"
{
#include "lua.h"
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
