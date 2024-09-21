#pragma once
#include <string>
#include <unordered_map>
extern "C"
{
#include "lua.h"
}

class ScriptApi;

ScriptApi &getApi(lua_State *L);
void registerLuaWorld(lua_State *L);
void setLuaApi(lua_State *L, ScriptApi *api);
void setPluginIndex(lua_State *L, size_t index);
