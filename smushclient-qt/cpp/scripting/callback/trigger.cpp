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
  , thread(plugin.state())
  , top(lua_gettop(thread.state()) + 1)
  , isValid(callback.findCallback(thread.state()))
  , nargs(callback.pushArguments(thread.state()))
{
}

CallbackTrigger::CallbackTrigger(CallbackTrigger&& other) noexcept
  : parent(std::move(other.parent))
  , plugin(other.plugin)
  , thread(std::move(other.thread))
  , top(other.top)
  , isValid(other.isValid)
  , nargs(other.nargs)
{
}

bool
CallbackTrigger::trigger()
{
  if (parent.isNull() || plugin->isDisabled() ||
      !thread.isChildOf(plugin->state())) [[unlikely]] {
    return false;
  }

  lua_State* L = thread.state();
  for (int i = top; i <= top + nargs; ++i) {
    lua_pushvalue(L, i);
  }

  return api_pcall(L, nargs, 0);
}
