#include "scriptstate.h"
#include <QtCore/QPointer>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>
#include "qlua.h"
#include "luaapi.h"
extern "C"
{
#include "lauxlib.h"
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_bit(lua_State *L);
  LUALIB_API int luaopen_lpeg(lua_State *L);
  LUALIB_API int luaopen_rex_pcre2(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
  LUALIB_API int luaopen_util(lua_State *L);
}

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

inline QString tr(const char *key)
{
  return QCoreApplication::translate("ScriptEngine", key);
}

static int panic(lua_State *L)
{
  const QString message = tr("PANIC: unprotected error in call to Lua API: %1");
  QErrorMessage::qtHandler()->showMessage(message.arg(qlua::getQString(L, -1)));
  return 0;
}

ScriptState::ScriptState(ScriptApi *api, const QString &pluginID)
    : L(luaL_newstate()),
      api(api),
      pluginID(pluginID)
{
  if (L == nullptr)
    throw std::bad_alloc();

  lua_atpanic(L, &panic);
  luaopen_bc(L);
  luaopen_bit(L);
  luaopen_lpeg(L);
  luaopen_rex_pcre2(L);
  luaopen_lsqlite3(L);
  luaopen_util(L);
  lua_settop(L, 0);
  registerLuaWorld(L);
  setLuaApi(L, api);
  api->setVariableMap(pluginID.toStdString(), createVariableMap(L));
}

ScriptState::ScriptState(ScriptState &&other)
    : L(other.L) {}

ScriptState::~ScriptState()
{
  api->unsetVariableMap(pluginID.toStdString(), getVariableMap(L));
  lua_close(L);
}

QString ScriptState::getError() const
{
  return qlua::getQString(L, -1);
}

RunScriptResult ScriptState::runScript(const QString &script)
{
  if (checkError(qlua::loadQString(L, script)))
    return RunScriptResult::CompileError;

  if (checkError(lua_pcall(L, 0, -1, 0)))
    return RunScriptResult::RuntimeError;

  return RunScriptResult::Ok;
}
