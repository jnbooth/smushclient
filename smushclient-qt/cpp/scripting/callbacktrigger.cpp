#include "callbacktrigger.h"
#include "lua/errors.h"
extern "C"
{
#include "lua.h"
}

CallbackTrigger::CallbackTrigger(lua_State* parentL, int nargs, QObject* parent)
  : nargs(nargs)
  , parent(parent)
  , thread(parentL)
  , top(lua_gettop(thread.state()) + 1)
{
  lua_xmove(parentL, thread.state(), nargs + 1);
}

CallbackTrigger::CallbackTrigger(CallbackTrigger&& other) noexcept
  : nargs(other.nargs)
  , parent(std::move(other.parent))
  , thread(std::move(other.thread))
  , top(other.top)
{
}

bool
CallbackTrigger::trigger()
{
  if (parent.isNull()) {
    return false;
  }

  lua_State* L = thread.state();
  for (int i = top; i <= top + nargs; ++i) {
    lua_pushvalue(L, i);
  }

  return api_pcall(L, nargs, 0);
}
