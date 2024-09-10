#include <QtCore/QPointer>
#include "luaapi.h"
#include "qlua.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
}

#define API_REG_KEY "smushclient.api"
#define WORLD_REG_KEY "smushclient.world"
#define WORLD_LIB_KEY "world"

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

static int L_ColourTell(lua_State *L)
{
  ScriptApi &api = getApi(L);
  int n = lua_gettop(L);
  for (int i = 1; i <= n; i += 3)
    api.ColourTell(
        qlua::getQColor(L, i),
        qlua::getQColor(L, i + 1),
        qlua::getQString(L, i + 2));

  return 0;
}

static const struct luaL_Reg worldlib[] =
    {{"ColourTell", L_ColourTell},

     {NULL, NULL}};

static const int worldlib_size = sizeof(worldlib) / sizeof(*worldlib) - 1;

static int L_world_tostring(lua_State *L)
{
  lua_pushliteral(L, "world");
  return 1;
}

static const struct luaL_Reg worldlib_meta[] = {{"__tostring", L_world_tostring}, {NULL, NULL}};

void registerLuaWorld(lua_State *L)
{
  lua_createtable(L, 0, worldlib_size);
  luaL_setfuncs(L, worldlib, 0);

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
