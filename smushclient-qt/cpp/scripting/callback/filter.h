#pragma once

class NamedPluginCallback;
struct lua_State;

class CallbackFilter
{
public:
  constexpr void operator|=(const CallbackFilter& other) noexcept
  {
    filter |= other.filter;
  }
  constexpr void clear() noexcept { filter = 0; }
  constexpr bool includes(unsigned int id) const noexcept
  {
    return (filter & id) != 0;
  }
  void scan(lua_State* L);

private:
  void setIfDefined(lua_State* L, const NamedPluginCallback& callback);

private:
  unsigned int filter = 0;
};
