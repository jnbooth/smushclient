#include "scriptstate.h"
#include <QtCore/QPointer>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>
#include "qlua.h"
#include "luaapi.h"
#include "lualibs.h"
extern "C"
{
#include "lauxlib.h"
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

ScriptState::ScriptState(ScriptApi *api)
    : L(luaL_newstate())
{
  if (L == nullptr)
    throw std::bad_alloc();

  lua_atpanic(L, &panic);
  openLuaLibs(L);
  registerLuaWorld(L);
  setLuaApi(L, api);
}

ScriptState::ScriptState(ScriptState &&other)
    : L(other.L) {}

ScriptState::~ScriptState()
{
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
