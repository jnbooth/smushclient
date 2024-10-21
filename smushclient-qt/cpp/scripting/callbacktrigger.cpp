#include "callbacktrigger.h"
#include "localization.h"
#include "luaapi.h"
#include "scriptapi.h"
extern "C"
{
#include "lua.h"
}

CallbackTrigger::CallbackTrigger(lua_State *L, int nargs)
    : L(L),
      nargs(nargs),
      top(lua_gettop(L) - nargs) {}

CallbackTrigger::CallbackTrigger(CallbackTrigger &&other)
    : L(other.L),
      nargs(other.nargs),
      top(other.top)
{
  other.moved = true;
}

CallbackTrigger::~CallbackTrigger()
{
  if (moved)
    return;
  lua_settop(L, top - 1);
}

bool CallbackTrigger::trigger()
{
  for (int i = top; i <= top + nargs; ++i)
    lua_pushvalue(L, i);

  if (lua_pcall(L, nargs, 0, 1) == LUA_OK) [[likely]]
    return true;

  getApi(L).printError(formatRuntimeError(L));
  return false;
}
