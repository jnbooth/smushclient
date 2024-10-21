#include "plugin.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>
#include "localization.h"
#include "luaapi.h"
#include "luaglobals.h"
#include "qlua.h"
#include "scriptapi.h"
#include "scriptthread.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_bit(lua_State *L);
  LUALIB_API int luaopen_cjson(lua_State *L);
  LUALIB_API int luaopen_lpeg(lua_State *L);
  LUALIB_API int luaopen_rex_pcre2(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
}
int luaopen_utils(lua_State *L);

using std::string;
using std::string_view;
using std::unordered_map;

constexpr int tracebackIdx = 1;
static const char *errorHandlerKey = "trace";

// Private Lua functions

static int L_panic(lua_State *L)
{
  const QString message = formatPanic(L);
  qCritical() << "panic(" << message << ")";
  QErrorMessage::qtHandler()->showMessage(message);
  return 0;
}

static int L_print(lua_State *L)
{
  const QString output = qlua::concatStrings(L);
  qInfo() << "print(" << output << ")";
  getApi(L).Tell(output);
  return 0;
}

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

inline bool api_pcall(lua_State *L, int nargs, int nreturn)
{
  if (checkError(lua_pcall(L, nargs, nreturn, tracebackIdx))) [[unlikely]]
  {
    getApi(L).printError(formatRuntimeError(L));
    lua_pop(L, 1);
    return false;
  }

  return true;
}

void setlib(lua_State *L, const char *name)
{
  lua_pushvalue(L, -1);
  lua_setglobal(L, name);
  lua_setfield(L, 1, name);
}

// Public methods

void Plugin::pushErrorHandler(lua_State *L)
{
  lua_rawgetp(L, LUA_REGISTRYINDEX, errorHandlerKey);
}

Plugin::Plugin(ScriptApi *api, PluginMetadata &&metadata)
    : L(luaL_newstate()),
      isDisabled(false),
      metadata(metadata)
{
  if (!L)
    throw std::bad_alloc();

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
  setlib(L, "json");
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
  setPluginIndex(L, metadata.index);
  setLuaApi(L, api);
  lua_settop(L, 0);
  lua_getglobal(L, LUA_DBLIBNAME);
  lua_getfield(L, -1, "traceback");
  lua_pushvalue(L, -1);
  lua_rawsetp(L, LUA_REGISTRYINDEX, errorHandlerKey);
  lua_remove(L, 1);
}

Plugin::Plugin(Plugin &&other)
    : L(other.L),
      isDisabled(other.isDisabled),
      metadata(std::move(other.metadata))
{
  other.moved = true;
}

Plugin::~Plugin()
{
  if (moved)
    return;
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

bool Plugin::hasFunction(const char *name) const
{
  const bool isFunction = lua_getglobal(L, name) == LUA_TFUNCTION;
  lua_pop(L, 1);
  return isFunction;
}

bool Plugin::runCallback(PluginCallback &callback) const
{
  if (!findCallback(callback))
    return false;
  if (!api_pcall(L, callback.pushArguments(L), callback.expectedSize()))
    return false;
  callback.collectReturned(L);
  return true;
}

bool Plugin::runCallbackThreaded(PluginCallback &callback) const
{
  if (!findCallback(callback))
    return false;
  const ScriptThread thread(L);
  lua_State *L2 = thread.state();
  lua_xmove(L, L2, 1);
  if (!api_pcall(L2, callback.pushArguments(L2), callback.expectedSize()))
    return false;
  callback.collectReturned(L2);
  return true;
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

// Private methods

bool Plugin::findCallback(const PluginCallback &callback) const
{
  if (isDisabled) [[unlikely]]
    return false;

  return callback.findCallback(L);
}
