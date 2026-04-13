#pragma once
#include <cstddef>

class ScriptApi;
struct lua_State;

int
registerLuaWorld(lua_State* L, size_t pluginIndex);
