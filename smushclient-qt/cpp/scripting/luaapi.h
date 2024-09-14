#pragma once

#include "scriptapi.h"
extern "C"
{
#include "lua.h"
}

std::unordered_map<std::string, std::string> *createVariableMap(lua_State *L);
std::unordered_map<std::string, std::string> *getVariableMap(lua_State *L);
void registerLuaWorld(lua_State *L);
void setLuaApi(lua_State *L, ScriptApi *api);
