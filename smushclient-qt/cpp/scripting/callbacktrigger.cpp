#include "callbacktrigger.h"
#include "errors.h"
extern "C"
{
#include "lua.h"
}

CallbackTrigger::CallbackTrigger(lua_State *parentL, int nargs)
    : moved(false),
      nargs(nargs),
      thread(parentL),
      L(thread.state()),
      top(lua_gettop(L) + 1)
{
  lua_xmove(parentL, L, nargs + 1);
}

CallbackTrigger::CallbackTrigger(CallbackTrigger &&other)
    : moved(false),
      nargs(other.nargs),
      thread(std::move(other.thread)),
      L(other.L),
      top(other.top)
{
  other.moved = true;
}

bool CallbackTrigger::trigger()
{
  for (int i = top; i <= top + nargs; ++i)
    lua_pushvalue(L, i);

  return api_pcall(L, nargs, 0);
}
