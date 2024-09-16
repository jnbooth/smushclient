#include <QtCore/QMetaProperty>
#include <QtCore/QPointer>
#include "luaapi.h"
#include "qlua.h"
#include "scriptenums.h"
#include "cxx-qt-gen/ffi.cxxqt.h"
#include "worldproperties.h"
extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
}

#define API_REG_KEY "smushclient.api"
#define VARIABLES_REG_KEY "smushclient.vars"
#define WORLD_REG_KEY "smushclient.world"
#define WORLD_LIB_KEY "world"

using std::string;
using stringmap = std::unordered_map<string, string>;

inline int returnCode(lua_State *L, ScriptReturnCode code)
{
  lua_pushnumber(L, (lua_Number)code);
  return 1;
}

template <typename T>
inline T *createUserdata(lua_State *L, const char *k)
{
  void *ud = lua_newuserdata(L, sizeof(T));
  lua_setfield(L, LUA_REGISTRYINDEX, k);
  return static_cast<T *>(ud);
}

template <typename T>
inline T *getUserdata(lua_State *L, const char *k)
{
  lua_getfield(L, LUA_REGISTRYINDEX, k);
  void *ud = lua_touserdata(L, -1);
  lua_pop(L, 1);
  return static_cast<T *>(ud);
}

void returnError(lua_State *L, const char *e)
{
  lua_pushstring(L, e);
  lua_error(L);
}

void setLuaApi(lua_State *L, ScriptApi *api)
{
  *createUserdata<QPointer<ScriptApi>>(L, API_REG_KEY) = api;
}

inline ScriptApi &getApi(lua_State *L)
{
  QPointer<ScriptApi> *ud = getUserdata<QPointer<ScriptApi>>(L, API_REG_KEY);
  if (ud == nullptr || ud->isNull())
    returnError(L, "Userdata was deleted");
  return *ud->data();
}

stringmap *createVariableMap(lua_State *L)
{
  stringmap *vars = createUserdata<stringmap>(L, VARIABLES_REG_KEY);
  *vars = stringmap();
  return vars;
}

inline stringmap *getVariableMap(lua_State *L)
{
  return getUserdata<stringmap>(L, VARIABLES_REG_KEY);
}

inline void pushVariable(lua_State *L, const stringmap &vars, const string &name)
{
  if (auto search = vars.find(name); search != vars.end())
    qlua::pushString(L, search->second);
  else
    lua_pushnil(L);
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

inline void insertTexts(lua_State *L, ScriptApi &api)
{
  int n = lua_gettop(L);
  for (int i = 1; i <= n; ++i)
    api.Tell(qlua::getQString(L, i));
}

static int L_ColourNameToRGB(lua_State *L)
{
  QColor color = qlua::getQColor(L, 0);
  if (color.isValid())
  {
    int r, g, b;
    color.getRgb(&r, &g, &b);
    lua_pushinteger(L, r << 16 | g << 8 | b);
  }
  else
    lua_pushinteger(L, 1);
  return 1;
}

static int L_ColourNote(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTextTriples(L, api);
  api.finishNote();
  return 0;
}

static int L_ColourTell(lua_State *L)
{
  insertTextTriples(L, getApi(L));
  return 0;
}

static int L_GetOption(lua_State *L)
{
  const QVariant option = getApi(L).GetOption(qlua::getString(L, 1));
  if (option.isValid())
    qlua::pushQVariant(L, option);
  else
    lua_pushinteger(L, -1);
  return 1;
}

static int L_GetOptionList(lua_State *L)
{
  qlua::pushStrings(L, WorldProperties::keys());
  return 1;
}

static int L_Note(lua_State *L)
{
  ScriptApi &api = getApi(L);
  insertTexts(L, api);
  api.finishNote();
  return 0;
}

static int L_SetOption(lua_State *L)
{
  getApi(L).SetOption(qlua::getString(L, 1), qlua::getQVariant(L, 2));
  return 1;
}

static int L_Tell(lua_State *L)
{
  insertTexts(L, getApi(L));
  return 0;
}

static int L_GetVariable(lua_State *L)
{
  pushVariable(L, *getVariableMap(L), qlua::getString(L, 1));
  return 1;
}

static int L_GetPluginVariable(lua_State *L)
{
  const string pluginID = qlua::getString(L, 1);
  const string name = qlua::getString(L, 2);
  const stringmap *vars = getApi(L).getVariableMap(pluginID);
  if (vars == nullptr)
    lua_pushnil(L);
  else
    pushVariable(L, *vars, name);
  return 1;
}

static int L_SetVariable(lua_State *L)
{
  (*getVariableMap(L))[qlua::getString(L, 1)] = qlua::getString(L, 2);
  return returnCode(L, ScriptReturnCode::OK);
}

static int L_Send(lua_State *L)
{
  return returnCode(L, getApi(L).Send(qlua::borrowBytes(L, 1)));
}

static const struct luaL_Reg worldlib[] =
    {{"ColourNameToRGB", L_ColourNameToRGB},
     {"ColourNote", L_ColourNote},
     {"ColourTell", L_ColourTell},
     {"GetOption", L_GetOption},
     {"GetOptionList", L_GetOptionList},
     {"GetPluginVariable", L_GetPluginVariable},
     {"GetVariable", L_GetVariable},
     {"Note", L_Note},
     {"Send", L_Send},
     {"Tell", L_Tell},
     {"SetOption", L_SetOption},
     {"SetVariable", L_SetVariable},

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
