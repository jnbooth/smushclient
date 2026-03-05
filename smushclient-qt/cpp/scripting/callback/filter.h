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
  constexpr void set(unsigned int id, bool on = true)
  {
    if (on) {
      filter |= id;
    } else {
      filter &= ~id;
    }
  }
  void scan(lua_State* L);

private:
  unsigned int filter = 0;
};
