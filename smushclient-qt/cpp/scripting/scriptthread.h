#pragma once

struct lua_State;

class ScriptThread
{
public:
  ScriptThread(lua_State *L);
  ScriptThread(ScriptThread &&other);
  ~ScriptThread();

  ScriptThread(const ScriptThread &) = delete;
  ScriptThread &operator=(const ScriptThread &) = delete;

  inline lua_State *state() const noexcept { return L; }

private:
  lua_State *L;
  bool moved = false;
  lua_State *parentL;
};
