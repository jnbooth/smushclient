#pragma once

#include "scriptapi.h"
extern "C"
{
#include "lua.h"
}

void registerLuaWorld(lua_State *L);
void setLuaApi(lua_State *L, ScriptApi *api);
