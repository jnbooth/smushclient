#include "trigger.h"
#include "../lua/errors.h"
#include "../plugin.h"
#include "plugincallback.h"
extern "C"
{
#include "lauxlib.h"
}

CallbackTrigger::CallbackTrigger(const Plugin& plugin,
                                 const PluginCallback& callback)
  : disabled(plugin.getDisabled())
  , thread(plugin.spawnThread())
{
  lua_State* L = thread.state();
  isValid = callback.findCallback(L);
  top = lua_gettop(L);
  nargs = callback.pushArguments(L);
}

CallbackTrigger::~CallbackTrigger()
{
  if (lua_State* L = thread.state(); L != nullptr && lua_gettop(L) >= top) {
    lua_remove(L, top);
  }
}

bool
CallbackTrigger::belongsToPlugin(const Plugin& plugin) const
{
  return plugin.getDisabled() == disabled;
}

bool
CallbackTrigger::trigger()
{
  if (*disabled) [[unlikely]] {
    return false;
  }
  lua_State* L = thread.state();
  if (L == nullptr) {
    return false;
  }
  luaL_checkstack(L, nargs + 1, nullptr);
  for (int i = top; i <= top + nargs; ++i) {
    lua_pushvalue(L, i);
  }

  return api_pcall(L, nargs, 0);
}
