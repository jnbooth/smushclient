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

static int panic(lua_State *L)
{
  const QString message = ScriptApi::tr("PANIC: unprotected error in call to Lua API: %1");
  QErrorMessage::qtHandler()->showMessage(message.arg(qlua::getError(L)));
  return 0;
}

Plugin::Plugin(ScriptApi *api, PluginMetadata &&metadata)
    : L(luaL_newstate()),
      isDisabled(false),
      metadata(metadata)
{
  if (L == nullptr)
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
  createVariableMap(L);
  setPluginID(L, metadata.id.toStdString());
  setLuaApi(L, api);
}

Plugin::Plugin(Plugin &&other)
    : L(other.L) {}

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

RunScriptResult Plugin::runScript(const QString &script) const
{
  if (isDisabled)
    return RunScriptResult::Disabled;

  if (checkError(qlua::loadQString(L, script)))
    return RunScriptResult::CompileError;

  if (checkError(lua_pcall(L, 0, -1, 0)))
    return RunScriptResult::RuntimeError;

  return RunScriptResult::Ok;
}
