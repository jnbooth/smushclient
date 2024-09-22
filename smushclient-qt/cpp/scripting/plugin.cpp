#include "plugin.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>
#include "luaapi.h"
#include "qlua.h"
#include "scriptapi.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_bit(lua_State *L);
  LUALIB_API int luaopen_lpeg(lua_State *L);
  LUALIB_API int luaopen_rex_pcre2(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
  LUALIB_API int luaopen_util(lua_State *L);
}

using std::string;
using std::string_view;
using std::unordered_map;

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

QString formatCompileError(lua_State *L)
{
  return ScriptApi::tr("Compile error: %1").arg(qlua::getError(L));
}

QString formatPanic(lua_State *L)
{
  return ScriptApi::tr("PANIC: unprotected error in call to Lua API: %1").arg(qlua::getError(L));
}

QString formatRuntimeError(lua_State *L)
{
  return ScriptApi::tr("Compile error: %1").arg(qlua::getError(L));
}

static int panic(lua_State *L)
{
  QErrorMessage::qtHandler()->showMessage(formatPanic(L));
  return 0;
}

inline bool api_pcall(lua_State *L, int nargs, int nreturn)
{
  if (checkError(lua_pcall(L, nargs, nreturn, 0))) [[unlikely]]
  {
    getApi(L).printError(formatRuntimeError(L));
    return false;
  }

  return true;
}

Plugin::Plugin(ScriptApi *api, PluginMetadata &&metadata)
    : L(luaL_newstate()),
      isDisabled(false),
      metadata(metadata)
{
  if (!L)
    throw std::bad_alloc();

  lua_atpanic(L, &panic);
  luaL_openlibs(L);
  luaopen_bc(L);
  luaopen_bit(L);
  luaopen_lpeg(L);
  luaopen_rex_pcre2(L);
  luaopen_lsqlite3(L);
  luaopen_util(L);
  lua_settop(L, 0);
  registerLuaWorld(L);
  setPluginIndex(L, metadata.index);
  setLuaApi(L, api);
}

Plugin::Plugin(Plugin &&other)
    : L(other.L),
      isDisabled(other.isDisabled),
      metadata(std::move(other.metadata)) {}

Plugin::~Plugin()
{
  lua_close(L);
}

void Plugin::disable()
{
  isDisabled = true;
}

void Plugin::enable()
{
  isDisabled = false;
}

QString Plugin::getError() const
{
  return qlua::getError(L);
}

bool Plugin::runCallback(string_view name, lua_Integer arg1, string_view arg2) const
{
  if (isDisabled) [[unlikely]]
    return false;

  lua_getglobal(L, name.data());
  lua_pushinteger(L, arg1);
  qlua::pushString(L, arg2);
  return api_pcall(L, 2, 0);
}

bool Plugin::runScript(const QString &script) const
{
  if (isDisabled) [[unlikely]]
    return false;

  if (checkError(qlua::loadQString(L, script))) [[unlikely]]
  {
    getApi(L).printError(formatCompileError(L));
    return false;
  }

  return api_pcall(L, 0, 0);
}
