#pragma once

#include <memory>
struct lua_State;

class ScriptThread
{
public:
  explicit ScriptThread(const std::shared_ptr<lua_State>& parentL);
  ScriptThread(ScriptThread&& other) noexcept;
  ~ScriptThread();

  ScriptThread(const ScriptThread&) = delete;
  ScriptThread& operator=(const ScriptThread&) = delete;
  ScriptThread& operator=(ScriptThread&&) = delete;

  lua_State* state() const noexcept { return parentL_.expired() ? nullptr : L; }

private:
  lua_State* L;
  std::weak_ptr<lua_State> parentL_;
};
