#pragma once
#include "scriptthread.h"

class PluginCallback;
struct lua_State;

class CallbackTrigger
{
public:
  CallbackTrigger(lua_State *L, int nargs);
  CallbackTrigger(CallbackTrigger &&other);

  CallbackTrigger(const CallbackTrigger &) = delete;
  CallbackTrigger &operator=(const CallbackTrigger &) = delete;

  bool trigger();

private:
  bool moved = false;
  int nargs;
  ScriptThread thread;
  lua_State *L;
  int top;
};
