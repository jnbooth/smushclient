#pragma once
#include <cstddef>

struct lua_State;

int
initLuaState(lua_State* L, size_t pluginIndex);
