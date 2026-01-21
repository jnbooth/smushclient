#pragma once

class NamedPluginCallback;
struct lua_State;

class CallbackFilter
{
public:
  constexpr void operator|=(const CallbackFilter& other)
  {
    filter |= other.filter;
  }
  constexpr void clear() { filter = 0; }
  constexpr bool includes(unsigned int id) const { return (filter & id) != 0; }
  void scan(lua_State* L);

private:
  void setIfDefined(lua_State* L, const NamedPluginCallback& callback);

private:
  unsigned int filter = 0;
};

#undef CALLBACK
