#include <QtCore/QPointer>
#include "luaapi.h"
#include "qlua.h"
#include "scriptenums.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
}

#define API_REG_KEY "smushclient.api"
#define WORLD_REG_KEY "smushclient.world"
#define WORLD_LIB_KEY "world"

inline int returnCode(lua_State *L, ScriptReturnCode code)
{
  lua_pushnumber(L, (lua_Number)code);
  return 1;
}

void setLuaApi(lua_State *L, ScriptApi *api)
{
  QPointer<ScriptApi> *ud = (QPointer<ScriptApi> *)lua_newuserdata(L, sizeof(QPointer<ScriptApi>));
  *ud = api;
  lua_setfield(L, LUA_REGISTRYINDEX, API_REG_KEY);
}

ScriptApi &getApi(lua_State *L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, API_REG_KEY);
  QPointer<ScriptApi> *ud = (QPointer<ScriptApi> *)lua_touserdata(L, -1);
  lua_pop(L, 1);
  ScriptApi *api = ud->data();
  return *api;
}

inline void insertTextTriples(lua_State *L, ScriptApi &api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; i += 3)
    api.ColourTell(
        qlua::getQColor(L, i),
        qlua::getQColor(L, i + 1),
        qlua::getQString(L, i + 2));
}

static int L_ColourNote(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTextTriples(L, api);
  api.insertBlock();
  return 0;
}

static int L_ColourTell(lua_State *L)
{
  insertTextTriples(L, getApi(L));
  return 0;
}

static int L_Send(lua_State *L)
{
  return returnCode(L, getApi(L).Send(qlua::borrowBytes(L, -1)));
}

static const struct luaL_Reg worldlib[] =
    {{"ColourNote", L_ColourNote},
     {"ColourTell", L_ColourTell},
     {"Send", L_Send},

     {NULL, NULL}};

static int L_world_tostring(lua_State *L)
{
  lua_pushliteral(L, "world");
  return 1;
}

static const struct luaL_Reg worldlib_meta[] = {{"__tostring", L_world_tostring}, {NULL, NULL}};

void registerLuaWorld(lua_State *L)
{
  luaL_newlib(L, worldlib);

  luaL_newmetatable(L, WORLD_REG_KEY);
  luaL_setfuncs(L, worldlib_meta, 0);
  lua_setmetatable(L, -2);

  lua_setglobal(L, WORLD_LIB_KEY);

  lua_pushglobaltable(L);
  if (lua_getmetatable(L, -1) == LUA_TNIL)
    lua_newtable(L);

  lua_getglobal(L, WORLD_LIB_KEY);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  lua_pop(L, 1);
}
