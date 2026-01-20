#pragma once

struct lua_State;

class ScriptThread
{
public:
  explicit ScriptThread(lua_State* L);
  ScriptThread(ScriptThread&& other) noexcept;
  ~ScriptThread();

  ScriptThread(const ScriptThread&) = delete;
  ScriptThread& operator=(const ScriptThread&) = delete;

  constexpr lua_State* state() const noexcept { return L; }

private:
  lua_State* L;
  lua_State* parentL;
};
