#pragma once
#include <cstddef>

class ScriptApi;
struct lua_State;

ScriptApi*
getApi(lua_State* L);
int
registerLuaWorld(lua_State* L);
int
setLuaApi(lua_State* L, ScriptApi* api);
int
setPluginIndex(lua_State* L, size_t index);
