#pragma once
#include <string>
#include <unordered_map>
extern "C"
{
#include "lua.h"
}

class ScriptApi;

std::unordered_map<std::string, std::string> *createVariableMap(lua_State *L);
std::unordered_map<std::string, std::string> *getVariableMap(lua_State *L);
void registerLuaWorld(lua_State *L);
void setLuaApi(lua_State *L, ScriptApi *api);
void setPluginID(lua_State *L, std::string_view pluginID);
