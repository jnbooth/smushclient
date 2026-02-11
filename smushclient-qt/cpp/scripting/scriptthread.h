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
  ScriptThread& operator=(ScriptThread&&) = delete;

  constexpr bool isChildOf(lua_State* state) const noexcept
  {
    return state == parentL;
  }
  constexpr lua_State* state() const noexcept { return L; }

private:
  lua_State* L;
  lua_State* parentL;
};
