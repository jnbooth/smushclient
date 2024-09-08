#include "scriptstate.h"
#include <QtCore/QCoreApplication>
#include <QtWidgets/QErrorMessage>
#include "luaq.h"

#define STACK 0

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
  QErrorMessage::qtHandler()->showMessage(message.arg(luaQ_toqstring(L, STACK - 1)));
  return 0;
}

ScriptState::ScriptState() : L(luaL_newstate())
{
  if (L == nullptr)
    throw std::bad_alloc();

  lua_atpanic(L, &panic);
}

ScriptState::ScriptState(ScriptState &&other) : L(other.L) {}

ScriptState::~ScriptState()
{
  lua_close(L);
}

QString ScriptState::getError() const
{
  return luaQ_toqstring(L, STACK - 1);
}

RunScriptResult ScriptState::runScript(const QString &script)
{
  if (checkError(luaQ_loadqstring(L, script)))
    return RunScriptResult::CompileError;

  if (checkError(lua_pcall(L, 0, STACK - 1, 0)))
    return RunScriptResult::RuntimeError;

  return RunScriptResult::Ok;
}
