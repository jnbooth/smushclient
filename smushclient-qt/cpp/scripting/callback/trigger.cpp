#include "trigger.h"
#include "../lua/errors.h"
#include "../plugin.h"
#include "plugincallback.h"
extern "C"
{
#include "lua.h"
}

CallbackTrigger::CallbackTrigger(const Plugin& plugin,
                                 const PluginCallback& callback,
                                 QObject* parent)
  : parent(parent)
  , plugin(&plugin)
  , thread(plugin.spawnThread())
{
  lua_State* L = thread.state();
  top = lua_gettop(L) + 1;
  isValid = callback.findCallback(L);
  nargs = callback.pushArguments(L);
}

bool
CallbackTrigger::trigger()
{
  if (parent.isNull() || plugin->isDisabled()) [[unlikely]] {
    return false;
  }
  lua_State* L = thread.state();
  if (L == nullptr) {
    return false;
  }
  for (int i = top; i <= top + nargs; ++i) {
    lua_pushvalue(L, i);
  }

  return api_pcall(L, nargs, 0);
}
