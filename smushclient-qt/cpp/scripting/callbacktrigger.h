#pragma once

struct lua_State;

class CallbackTrigger
{
public:
  CallbackTrigger(lua_State *L, int nargs);
  CallbackTrigger(CallbackTrigger &&other);
  ~CallbackTrigger();

  CallbackTrigger(const CallbackTrigger &) = delete;
  CallbackTrigger &operator=(const CallbackTrigger &) = delete;

  bool trigger();

private:
  lua_State *L;
  bool moved;
  int nargs;
  int top;
};
